#ifndef CORE_DECL_H_
#define CORE_DECL_H_

template <typename T>
class Core
{
   public:
    Core();
    ~Core();

    void execute_one_inst();
    void reset();

    void debug_get_reg_val();

   private:
    void single_instruction();
};

#endif  // CORE_DECL_H_