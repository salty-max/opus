#include <opus/opus.hpp>

int main() {
    const opus::Logger log{opus::stderr_sink(), opus::LogLevel::Debug};
    log.info("opus sandbox, engine v{}", opus::version_string);
    return 0;
}
