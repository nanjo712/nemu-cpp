#ifndef CORE_DECL_H_
#define CORE_DECL_H_

#include <string_view>
template <typename T>
class Core
{
   public:
    Core();
    ~Core();

    void execute_one_inst();
    void reset();

    auto debug_get_reg_index(std::string_view reg_num);
    auto debug_get_reg_val(int reg_num);
    auto debug_get_pc();

   private:
    void single_instruction();
};

template <typename T>
concept CoreType = std::is_base_of<Core<T>, T>::value;

#endif  // CORE_DECL_H_