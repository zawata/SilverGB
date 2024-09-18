#include "log.hpp"
 Silver::Logger &Silver::getLogger() {
    static Silver::Logger global_logger = Silver::Logger();

    return global_logger;
}
