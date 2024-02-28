#include "ddbase/stdafx.h"
#include "ddbase/str/ddtree.h"
#include <queue>
#include <stack>

namespace NSP_DD {
std::shared_ptr<ddtree> ddtree::clone()
{
    std::queue<ddtree*> srcs;
    std::queue<ddtree*> dsts;
    std::shared_ptr<ddtree> sjson(new ddtree());
    std::shared_ptr<ddtree> return_value(new ddtree());
    srcs.push(this);
    dsts.push(return_value.get());
    while (!srcs.empty()) {
        ddtree* src = srcs.front();
        srcs.pop();
        ddtree* dst = dsts.front();
        dsts.pop();
        dst->m_str = src->m_str;
        dst->m_childs.resize(src->m_childs.size());
        for (size_t i = 0; i < src->m_childs.size(); ++i) {
            srcs.push(src->m_childs[i].get());
            dst->m_childs[i] = std::shared_ptr<ddtree>(new ddtree());
            dsts.push(dst->m_childs[i].get());
        }
    }
    return return_value;
}

std::string ddtree::to_string()
{
    if (m_str.empty()) {
        return "";
    }

    struct stack_context {
        ddtree* self = nullptr;
        std::vector<ddtree*> childs;
        size_t childs_index = 0;
        static stack_context* create(ddtree* sjson)
        {
            auto* stack = new stack_context();
            stack->self = sjson;
            stack->childs.resize(sjson->m_childs.size());
            for (size_t i = 0; i < sjson->m_childs.size(); ++i) {
                stack->childs[i] = sjson->m_childs[i].get();
            }
            stack->childs_index = 0;
            return stack;
        }
    };

    std::string return_value;
    u32 level = 0;
    std::stack<stack_context*> stacks;
    stacks.push(stack_context::create(this));

    while (!stacks.empty()) {
        auto* context = stacks.top();
        if (context->self != nullptr) {
            auto* self = context->self;
            if (self->m_str.empty()) {
                delete context;
                stacks.pop();
                --level;
                continue;
            }

            return_value += (std::string(level * 2, ' ') + self->m_str);
            if (self->m_childs.empty()) {
                return_value += "\n";
                delete context;
                stacks.pop();
                --level;
                continue;
            }
            return_value += " {\n";
            context->self = nullptr;
        }

        if (context->childs_index < context->childs.size()) {
            ++level;
            stacks.push(stack_context::create(context->childs[context->childs_index++]));
            continue;
        }

        return_value += (std::string(level * 2, ' ') + "}\n");
        delete context;
        stacks.pop();
        --level;
    }
    return return_value;
}

std::shared_ptr<ddtree> ddtree::from_string(const std::string&)
{
    DDASSERT_FMT(false, L"TODO");
    return nullptr;
}
} // namespace NSP_DD
