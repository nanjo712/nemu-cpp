#include <getopt.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

#include "Debugger/Debugger.hpp"
#include "ISA/riscv32/EmuCore.hpp"
#include "Memory/Memory.h"
#include "Monitor/Monitor.hpp"

std::string log_file;
std::string diff_so_file;
int difftest_port;
bool is_batch_mode = false;
bool is_diff = false;

class Nemu
{
   public:
    Nemu(int argc = 0, char* argv[] = nullptr)
    {
        spdlog::info("Build time: {}, {}", __TIME__, __DATE__);
        parse_args(argc, argv);
        spdlog::info("Welcome to NEMU!");
        spdlog::info("For help, type \"help\"");

        Memory memory;
        RISCV32::EmuCore core(memory);
        Monitor monitor(core, memory);
        Debugger debugger(monitor);
        debugger.run(is_batch_mode);
    }
    ~Nemu() { spdlog::info("Exit NEMU"); }

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
                    // Monitor::elf_file = optarg;
                    break;
                case 1:
                    // ISA_Wrapper::img_file = optarg;
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

int main(int argc, char* argv[]) { Nemu nemu(argc, argv); }