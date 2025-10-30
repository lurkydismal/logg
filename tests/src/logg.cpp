#include "logg.hpp"

#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include "test.hpp"

struct fdRedirect {
    // start redirecting `fd` (STDOUT_FILENO or STDERR_FILENO)
    void start( int _fd ) {
        _targetFd = _fd;
        if ( pipe( _pipeFds.data() ) != 0 ) {
            throw std::runtime_error( "pipe() failed" );
        }
        _pipeRead = _pipeFds[ 0 ];
        _pipeWrite = _pipeFds[ 1 ];

        // save original
        _savedFd = dup( _targetFd );
        if ( _savedFd == -1 ) {
            close( _pipeRead );
            close( _pipeWrite );
            throw std::runtime_error( "dup() failed" );
        }

        // replace target fd with pipe write end
        if ( dup2( _pipeWrite, _targetFd ) == -1 ) {
            // cleanup
            close( _pipeRead );
            close( _pipeWrite );
            close( _savedFd );
            throw std::runtime_error( "dup2() failed" );
        }

        // we can close the extra write fd; target_fd is the live handle now
        close( _pipeWrite );
        _pipeWrite = -1;
    }

    // stop redirecting and return captured string
    auto stopAndRead() -> std::string {
        // flush C/C++ streams to try to get everything out
        fflush( nullptr );
        std::cout.flush();
        std::cerr.flush();

        // restore original fd: this closes the current target_fd which causes
        // the pipe to get EOF on read once writers are done.
        if ( _savedFd != -1 ) {
            if ( dup2( _savedFd, _targetFd ) == -1 ) {
                // continue anyway to try reading
            }
            close( _savedFd );
            _savedFd = -1;
        }

        // read available bytes from the pipe
        std::string l_out;
        constexpr size_t l_bufsize = 4096;
        std::vector< char > l_buf( l_bufsize );

        // Read until EOF (read returns 0)
        ssize_t l_r = 0;
        while ( ( l_r = ::read( _pipeRead, l_buf.data(), l_bufsize ) ) > 0 ) {
            l_out.append( l_buf.data(), static_cast< size_t >( l_r ) );
        }

        if ( _pipeRead != -1 ) {
            close( _pipeRead );
            _pipeRead = -1;
        }

        return l_out;
    }

private:
    std::array< int, 2 > _pipeFds{ -1, -1 };
    int _pipeRead{ -1 };
    int _pipeWrite{ -1 };
    int _savedFd{ -1 };
    int _targetFd{ -1 };
};

class LogFixture : public testing::Test {
protected:
    void TearDown() override {
        // Nothing global to clean in log.hpp itself — but ensure std streams
        // flushed.
        fflush( nullptr );
        std::cout.flush();
        std::cerr.flush();
    }
};

TEST_F( LogFixture, InfoWritesToStdout ) {
    fdRedirect l_out;
    l_out.start( STDOUT_FILENO );

    logg::info( "Hello {}!", "world" );

    const std::string l_captured = l_out.stopAndRead();

    // Basic sanity checks
    EXPECT_NE( l_captured.find( "INFO:" ), std::string::npos )
        << "Captured: " << l_captured;
    EXPECT_NE( l_captured.find( "Hello world!" ), std::string::npos )
        << "Captured: " << l_captured;
}

TEST_F( LogFixture, WarningWritesToStderr ) {
    fdRedirect l_err;
    l_err.start( STDERR_FILENO );

    logg::warning( "Watch out: {}", 123 );

    const std::string l_captured = l_err.stopAndRead();

    EXPECT_NE( l_captured.find( "WARNING:" ), std::string::npos )
        << "Captured: " << l_captured;
    EXPECT_NE( l_captured.find( "Watch out: 123" ), std::string::npos )
        << "Captured: " << l_captured;
}

TEST_F( LogFixture, ErrorWritesToStderr ) {
    fdRedirect l_err;
    l_err.start( STDERR_FILENO );

    logg$error( "This is an {}: {}", "error", 77 );

    const std::string l_captured = l_err.stopAndRead();

    EXPECT_NE( l_captured.find( "ERROR:" ), std::string::npos )
        << "Captured: " << l_captured;
    EXPECT_NE( l_captured.find( "This is an error: 77" ), std::string::npos )
        << "Captured: " << l_captured;
}

#ifdef DEBUG
TEST_F( LogFixture, DebugWritesToStdout ) {
    fdRedirect l_out;
    l_out.start( STDOUT_FILENO );

    logg$debug( "Dbg: {} {}", 1, "two" );

    const std::string l_captured = l_out.stopAndRead();

    EXPECT_NE( l_captured.find( "DEBUG:" ), std::string::npos )
        << "Captured: " << l_captured;
    EXPECT_NE( l_captured.find( "Dbg: 1 two" ), std::string::npos )
        << "Captured: " << l_captured;
}

TEST_F( LogFixture, VariableMacroPrintsVariable ) {
    fdRedirect l_out;
    l_out.start( STDOUT_FILENO );

    int l_testValue = 1234;
    logg$variable(
        l_testValue ); // macro expands to _variable which uses _debug

    const std::string l_captured = l_out.stopAndRead();

    EXPECT_NE( l_captured.find( "DEBUG:" ), std::string::npos )
        << "Captured: " << l_captured;
    // variable prints like: "<name> = '1234'"
    EXPECT_NE( l_captured.find( "l_testValue" ), std::string::npos )
        << "Captured: " << l_captured;
    EXPECT_NE( l_captured.find( "1234" ), std::string::npos )
        << "Captured: " << l_captured;
}
#else
// If DEBUG is not defined, debug and variable are no-ops. Provide a light test
// that calling debug() doesn't crash (no output expected).
TEST_F( LogFixture, DebugIsNoopWhenNotDefined ) {
    // call it to ensure no crash; nothing to capture because it's a no-op
    logg::debug( "should not appear: {}", 1 );
    SUCCEED();
}
#endif
