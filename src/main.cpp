#include <exception>

#include "core/Application.h"
#include "core/Log.h"

int main(int argc, char** argv) {
    try {
        vlt::Application application(argc, argv);
        return application.run();
    } catch (const std::exception& error) {
        VLT_LOG_ERROR("Fatal error: %s", error.what());
        return 1;
    } catch (...) {
        VLT_LOG_ERROR("Fatal error: unknown exception.");
        return 1;
    }
}
