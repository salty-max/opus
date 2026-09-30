#include <opus/opus.hpp>

int main() {
    const opus::Logger log{opus::stderr_sink()};
    log.info("consumer linked against opus {}", opus::version_string);
    return 0;
}
