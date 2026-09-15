#include <iostream>
#include <JANA/JApplication.h>
#include <JANA/CLI/JMain.h>

extern "C" void InitPlugin(JApplication* app);

int main(int argc, char* argv[]) {
    auto options = jana::ParseCommandLineOptions(argc, argv, true);
    auto app = jana::CreateJApplication(options);
    InitPlugin(app);
    return jana::Execute(app, options);
}
