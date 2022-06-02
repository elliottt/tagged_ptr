#include "doctest/doctest.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <type_traits>

#include "tagged/unique_ptr.h"

using namespace std::literals::string_view_literals;

class Var;
class Lit;
class Binop;

class ExprPtr final : public tagged::UniquePtr<ExprPtr, Var, Lit, Binop> {
public:
    using UniquePtr::UniquePtr;

    std::string show() const;

    enum class Tag {
        None = 0, // this case exists for the null pointer
        Var = tag_of<Var>(),
        Lit = tag_of<Lit>(),
        Binop = tag_of<Binop>(),
    };

    // Keep `UniquePtr::tag` hidden, and expose a typed interface to the tag instead. This way we can get exhaustiveness
    // checking everywhere the `tag` function is used.
    Tag tag() const {
        return static_cast<Tag>(UniquePtr::tag());
    }
};

static_assert(std::is_swappable<ExprPtr>::value);

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
    case Tag::Var:
        return this->cast<Var>().show();

    case Tag::Lit:
        return this->cast<Lit>().show();

    case Tag::Binop:
        return this->cast<Binop>().show();

    default:
        return "";
    }
}

using Env = std::unordered_map<std::string, int32_t>;

ExprPtr simplify(Env &env, ExprPtr e) {
    switch (e.tag()) {

    case ExprPtr::Tag::Var: {
        auto &var = e.cast<Var>();
        auto it = env.find(var.name);
        if (it != env.end()) {
            return ExprPtr::make<Lit>(it->second);
        }
        break;
    }

    case ExprPtr::Tag::Lit:
        break;

    case ExprPtr::Tag::Binop: {
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

    case ExprPtr::Tag::None:
        break;
    }

    return e;
}

class Increment {
public:
    int &count;

    Increment(int &count) : count{count} {}
    ~Increment() {
        count++;
    }
};

class SimplePtr final : public tagged::UniquePtr<SimplePtr, Increment> {
public:
    using UniquePtr::UniquePtr;
};

static_assert(std::is_swappable<SimplePtr>::value);

SimplePtr pass_through(SimplePtr p) {
    return p;
}

TEST_CASE("UniquePtr") {
    auto var = ExprPtr::make<Var>("x");
    CHECK_EQ(1, ExprPtr::tag_of<Var>());

    auto e = ExprPtr::make<Binop>(Binop::Op::Add, std::move(var), ExprPtr::make<Lit>(5));
    CHECK_EQ("(x + 5)", e.show());

    Env env{{"x", 5}};
    e = simplify(env, std::move(e));
    CHECK_EQ("10", e.show());

    SUBCASE("move assignment") {
        int count = 0;
        auto x = SimplePtr::make<Increment>(count);
        x = std::move(x);
        CHECK_EQ(0, count);
    }

}
