#include "doctest/doctest.h"
#include <cstdint>
#include <string>
#include <unordered_map>

#include "tagged/unique_ptr.h"

using namespace std::literals::string_view_literals;

class Var;
class Lit;
class Binop;

class ExprPtr final : public tagged::UniquePtr<ExprPtr, Var, Lit, Binop> {
public:
    using UniquePtr::UniquePtr;

    std::string show() const;
};

class Var final {
public:
    std::string name;

    explicit Var(std::string name) : name{std::move(name)} {}

    std::string show() const {
        return this->name;
    }
};

class Lit final {
public:
    int32_t val;

    explicit Lit(int32_t val) : val{val} {}

    std::string show() const {
        return std::to_string(val);
    }
};

class Binop final {
public:
    enum class Op {
        Add,
        Mul,
    } op;

    ExprPtr left;
    ExprPtr right;

    Binop(Op op, ExprPtr left, ExprPtr right) : op{op}, left{std::move(left)}, right{std::move(right)} {}

    static std::string_view op_str(Op op) {
        switch (op) {
        case Op::Add:
            return "+";

        case Op::Mul:
            return "*";
        }
    }

    std::string show() const {
        std::string res;

        res += "("sv;
        res += left.show();
        res += " "sv;
        res += op_str(this->op);
        res += " "sv;
        res += right.show();
        res += ")"sv;

        return res;
    }
};

std::string ExprPtr::show() const {
    switch (this->tag()) {
    case tag_of<Var>():
        return this->cast<Var>().show();

    case tag_of<Lit>():
        return this->cast<Lit>().show();

    case tag_of<Binop>():
        return this->cast<Binop>().show();

    default:
        return "";
    }
}

using Env = std::unordered_map<std::string, int32_t>;

ExprPtr simplify(Env &env, ExprPtr e) {
    switch (e.tag()) {

    case ExprPtr::tag_of<Var>(): {
        auto &var = e.cast<Var>();
        auto it = env.find(var.name);
        if (it != env.end()) {
            return ExprPtr::make<Lit>(it->second);
        }
        break;
    }

    case ExprPtr::tag_of<Lit>():
        break;

    case ExprPtr::tag_of<Binop>(): {
        auto &binop = e.cast<Binop>();
        binop.left = simplify(env, std::move(binop.left));
        binop.right = simplify(env, std::move(binop.right));

        if (binop.left.isa<Lit>() && binop.right.isa<Lit>()) {
            auto lval = binop.left.cast<Lit>().val;
            auto rval = binop.right.cast<Lit>().val;
            switch (binop.op) {
            case Binop::Op::Add:
                return ExprPtr::make<Lit>(lval + rval);

            case Binop::Op::Mul:
                return ExprPtr::make<Lit>(lval * rval);
            }
        }

        break;
    }
    }

    return e;
}

TEST_CASE("UniquePtr") {
    auto var = ExprPtr::make<Var>("x");
    CHECK_EQ(1, ExprPtr::tag_of<Var>());

    auto e = ExprPtr::make<Binop>(Binop::Op::Add, std::move(var), ExprPtr::make<Lit>(5));
    CHECK_EQ("(x + 5)", e.show());

    Env env{{"x", 5}};
    e = simplify(env, std::move(e));
    CHECK_EQ("10", e.show());
}
