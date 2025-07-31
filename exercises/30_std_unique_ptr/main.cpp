#include "../exercise.h"
#include <cstring>
#include <memory>
#include <string>
#include <vector>

// READ: `std::unique_ptr` <https://zh.cppreference.com/w/cpp/memory/unique_ptr>
/*
核心考察点是理解 std::unique_ptr 资源的生命周期与析构过程，以及传递（移动）智能指针时对象的状态变化，
结合析构函数中记录字符串 _records 以 'r', 'd', 'f' 代表不同操作记录资源的生命周期。
*/

std::vector<std::string> RECORDS;

class Resource {
    std::string _records;

public:
    void record(char record) {
        _records.push_back(record);
    }

    ~Resource() {
        RECORDS.push_back(_records); 
        /*
        当 Resource 对象析构时，将 _records 追加到全局 RECORDS 中。
        */
    }
};

/*
reset/drop/forward 三个函数接受并返回 Unique（std::unique_ptr<Resource>）：
    reset 记录 'r'，返回新 Resource。
    drop 记录 'd'，返回空指针（销毁当前对象）。
    forward 记录 'f'，返回原指针。
*/
using Unique = std::unique_ptr<Resource>;
/*
reset(ptr)	如果 ptr 不为空，则对其记录 'r'，然后丢弃并返回新的 Resource 对象
drop(ptr)	如果 ptr 不为空，则对其记录 'd'，然后销毁资源，返回 nullptr
forward(ptr)	如果 ptr 不为空，则记录 'f'，并返回同一个 ptr（不修改指针）
每个 Resource 对象在被销毁时，会将其记录字符串 _records 推入全局 RECORDS 向量，形成一个 std::string。
*/
Unique reset(Unique ptr) {
    if (ptr) ptr->record('r');
    return std::make_unique<Resource>();
}
Unique drop(Unique ptr) {
    if (ptr) ptr->record('d');
    return nullptr;
}
Unique forward(Unique ptr) {
    if (ptr) ptr->record('f');
    return ptr;
}

int main(int argc, char **argv) {
    std::vector<std::string> problems[3];

    drop(forward(reset(nullptr)));
    problems[0] = std::move(RECORDS);
    /*
    运算顺序（从内向外）：
    reset(nullptr)：
        入参为 nullptr，所以不记录 'r'
        创建了一个新的 Resource 实例 R1
    forward(R1)：
        R1 记录 'f'
        返回 R1
    drop(R1)：
        R1 记录 'd'
        销毁 R1 → RECORDS.push_back("fd")
✔️ 最终：answers[0] = {"fd"}
    */

    forward(drop(reset(forward(forward(reset(nullptr))))));
    problems[1] = std::move(RECORDS);
    /*
    运算顺序：
    reset(nullptr) → 创建新资源 R1（无记录）
    forward(R1) → R1 记录 'f'，返回 R1
    forward(R1) → R1 记录 'f'，返回 R1
    reset(R1)：
        对 R1 记录 'r'
        析构 R1（此时它已有记录 "ffr"，入 RECORDS）
        创建新的 R2，返回 R2
    drop(R2)：
        R2 记录 'd'
        销毁 R2（记录 "d"，入 RECORDS）
    forward(nullptr) → 跳过
✔️ 最终：answers[1] = {"ffr", "d"}

    如果 ptr == nullptr，什么都不做，直接 return nullptr
    所以：在执行 forward(nullptr) 时，相当于什么都没发生
    没有资源被操作、记录、或者销毁，所以 不会生成任何新记录
✅ 这个跳过是“逻辑跳过”而不是语法跳过，它确实调用了函数，只是因为输入是 nullptr，它什么都没干。

    forward
    └── drop
        └── reset
            └── forward
                └── forward
                    └── reset(nullptr)
                        └── return R1
    → forward(R1) —记录 f
    → forward(R1) —记录 f
    → reset(R1)   —记录 r，销毁（"ffr"）
    → create R2
    → drop(R2)    —记录 d，销毁（"d"）
    → forward(nullptr) 跳过
    */

    drop(drop(reset(drop(reset(reset(nullptr))))));
    problems[2] = std::move(RECORDS);
    /*
    运算顺序：
    reset(nullptr) → 创建新资源 R1（无记录）
    reset(R1)：
        R1 记录 'r'
        销毁 R1（记录 "r"，入 RECORDS）
        返回新资源 R2
    drop(R2)：
        R2 记录 'd'
        销毁 R2（记录 "d"，入 RECORDS）
        返回 nullptr
    reset(nullptr) → 创建新资源 R3（无记录）
    drop(R3)：
        R3 记录 'd'
        销毁 R3（记录 "d"，入 RECORDS）
    drop(nullptr) → 跳过
✔️ 最终：answers[2] = {"r", "d", "d"}

    R1->record('r')：记录 'r'
    R1 被销毁，调用析构函数，把 "r" 加入 RECORDS
    然后 reset() 函数内部又用 std::make_unique<Resource>() 创建新的 Resource 实例 R2
    返回这个新实例指针 R2
⚠️ 注意：虽然函数名叫 reset，它内部实际销毁旧的资源并 返回新的 Resource 实例，所以 "创建新的 R2"。
    */

    /*
    总结通用分析技巧：
    从最内层向外分析调用顺序
    每一个 reset(ptr) 如果 ptr 不为 null，就记录 'r' 并触发销毁
    每一个 drop(ptr) 都是销毁的触发点，会把记录 push 到 RECORDS
    每一次返回的新 Resource，都从空记录开始
    每个最终的 std::string 是 一个 Resource 生命周期的完整记录
    */
    // ---- 不要修改以上代码 ----

    std::vector<const char *> answers[]{
        //{"fd"},
        // TODO: 分析 problems[1] 中资源的生命周期，将记录填入 `std::vector`
        // NOTICE: 此题结果依赖对象析构逻辑，平台相关，提交时以 CI 实际运行平台为准
        // {"", "", "", "", "", "", "", ""},
        // {"", "", "", "", "", "", "", ""},
        {"fd"},               // problems[0]
        {"ffr", "d"},         // problems[1]
        {"r", "d", "d"}, // ✅ fixed answers[2]
    };

    // ---- 不要修改以下代码 ----

    for (auto i = 0; i < 3; ++i) {
        std::cout << "problems[" << i << "].size(): " << problems[i].size() << std::endl;
        std::cout << "answers[" << i << "].size(): " << answers[i].size() << std::endl;
        std::cout << "problems[2].size() = " << problems[2].size() << std::endl;
        std::cout << "answers[2].size() = " << answers[2].size() << std::endl;
        std::cout << "Adding answer #" << i << " to answers[2]\n";

        ASSERT(problems[i].size() == answers[i].size(), "wrong size");
        for (auto j = 0; j < problems[i].size(); ++j) { 
            std::cout << "problems[" << i << "][" << j << "]: \"" << problems[i][j] << "\", length: " << problems[i][j].size() << '\n';
            std::cout << "answers[" << i << "][" << j << "]: \"" << answers[i][j] << "\", strlen: " << std::strlen(answers[i][j]) << '\n';
            if (std::strcmp(problems[i][j].c_str(), answers[i][j]) != 0) {
                std::cout << ">>> Mismatch detected!\n";
            }
            std::cout << "Comparing problems[" << i << "][" << j << "]: \"" << problems[i][j] << "\" with answers[" << i << "][" << j << "]: \"" << answers[i][j] << "\"\n";
            //遍历 problems[i] 中的每个元素。这里假设 problems 是一个二维的 std::vector<std::string>，也就是 problems[i] 是一个字符串组成的向量。
            std::cout << "Comparing problems[" << i << "][" << j << "]: " << "\"" << problems[i][j] << "\" vs \"" << answers[i][j] << "\"" << std::endl;
            ASSERT(std::strcmp(problems[i][j].c_str(), answers[i][j]) == 0, "wrong location"); 
            // 逐个比较 problems[i] 与 answers[i] 中的字符串元素是否一致。
            /*
            这行的作用是断言判断：problems[i][j].c_str()：把 std::string 转换成 const char*。
            answers[i][j]：假设是一个二维的 const char* 数组或类似结构。
            std::strcmp(...) == 0：表示两个 C 风格字符串内容完全相同。如果不相同，就触发 ASSERT 报错，并提示 "wrong location"。
            */
        }
    }

    return 0;
}
/*
decltype 用于推断表达式类型，在复杂智能指针和模板代码中避免手写冗余类型。
理解对象的生命周期及智能指针操作对性能和内存安全至关重要，尤其在高频交易系统中，避免内存泄漏和未定义行为。
这题实战考察了智能指针移动、析构调用和资源管理的理解。
*/