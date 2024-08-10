#include <getopt.h>
#include <spdlog/spdlog.h>

#include <string>

#include "Debugger/Debugger.h"
#include "ISA/ISA_Wrapper.h"
#include "Monitor/Monitor.h"

std::string log_file;
std::string diff_so_file;
int difftest_port;
bool is_batch_mode = false;
bool is_diff = false;

class Nemu
{
   public:
    Nemu(int argc, char* argv[])
    {
        spdlog::info("Build time: {}, {}", __TIME__, __DATE__);
        parse_args(argc, argv);
        Debugger& debugger = Debugger::getDebugger();
        spdlog::info("Welcome to NEMU!");
        spdlog::info("For help, type \"help\"");
        debugger.run(is_batch_mode);
    }
    ~Nemu() { spdlog::info("Exit nemu"); }

   private:
    int parse_args(int argc, char* argv[])
    {
        const struct option table[] = {
            {"batch", no_argument, NULL, 'b'},
            {"log", required_argument, NULL, 'l'},
            {"diff", required_argument, NULL, 'd'},
            {"port", required_argument, NULL, 'p'},
            {"elf", required_argument, NULL, 'e'},
            {"help", no_argument, NULL, 'h'},
            {0, 0, NULL, 0},
        };
        int o;
        while ((o = getopt_long(argc, argv, "-bhl:d:p:e:", table, NULL)) != -1)
        {
            switch (o)
            {
                case 'b':
                    is_batch_mode = true;
                    break;
                case 'p':
                    sscanf(optarg, "%d", &difftest_port);
                    break;
                case 'l':
                    log_file = optarg;
                    break;
                case 'd':
                    diff_so_file = optarg;
                    break;
                case 'e':
                    Monitor::elf_file = optarg;
                    break;
                case 1:
                    ISA_Wrapper::img_file = optarg;
                    return 0;
                default:
                    print_usage();
            }
        }
        return 0;
    }
    void print_usage()
    {
        printf("Usage: nemu [OPTION...] IMAGE [args]\n\n");
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf(
            "\t-d,--diff=REF_SO        run DiffTest with reference "
            "REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
};

int main(int argc, char* argv[])
{
    Nemu nemu(argc, argv);
    return Monitor::getMonitor().is_bad_status();
}