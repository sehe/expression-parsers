#pragma once
#include <boost/spirit/home/x3.hpp>

namespace PhoeniX3 {
    namespace x3 = boost::spirit::x3;

    // base facilities
    void eval(void); // ADL enable

    // for instance-only operator overloads
    template <typename T>
    struct _base_type {
        using self = T;
        using base = _base_type;
        template <typename U> auto operator=(U&&) const;
        template <typename Ctx> decltype(auto) operator()(Ctx& ctx)/*&&*/ {
            return eval(ctx, self{});
        }
    };

    // placeholders
    struct _val_type  : _base_type<_val_type>  {using base::operator(); using base::operator=;};
    struct _attr_type : _base_type<_attr_type> {using base::operator(); using base::operator=;};

    namespace placeholders {
        _val_type    static const _val  {};
        _attr_type   static const _attr {};
    }

    // expression types
    template <typename L, typename R, typename Op> struct BinExpr : _base_type<BinExpr<L,R,Op> > { 
        using BinExpr::base::operator(); 
        using BinExpr::base::operator=;
    };

    namespace tag {
        struct _add; struct _add_assign;
        struct _sub; struct _sub_assign;
        struct _mul; struct _mul_assign;
        struct _div; struct _div_assign;
        struct _exp; struct _exp_assign;
        struct _assign;
    }

    template <typename L, typename R> BinExpr<L, R, tag::_exp_assign> operator^=(L&&, R&&) { return {}; }
    template <typename L, typename R> BinExpr<L, R, tag::_add_assign> operator+=(L&&, R&&) { return {}; }
    template <typename L, typename R> BinExpr<L, R, tag::_sub_assign> operator-=(L&&, R&&) { return {}; }
    template <typename L, typename R> BinExpr<L, R, tag::_mul_assign> operator*=(L&&, R&&) { return {}; }
    template <typename L, typename R> BinExpr<L, R, tag::_div_assign> operator/=(L&&, R&&) { return {}; }
    template <typename L, typename R> BinExpr<L, R, tag::_exp> operator&(L&&, R&&) { return {}; }
    template <typename L, typename R> BinExpr<L, R, tag::_sub> operator-(L&&, R&&) { return {}; }
    template <typename L, typename R> BinExpr<L, R, tag::_mul> operator*(L&&, R&&) { return {}; }
    template <typename L, typename R> BinExpr<L, R, tag::_div> operator/(L&&, R&&) { return {}; }
    template <typename L> template <typename R>
        auto _base_type<L>::operator=(R&&) const { return BinExpr<L, R, tag::_assign>{}; }

    template <typename Ctx>        auto&& eval(Ctx& ctx, _val_type) { return x3::_val(ctx); }
    template <typename Ctx>        auto&& eval(Ctx& ctx, _attr_type) { return x3::_attr(ctx); }

    template <typename L, typename R, typename Ctx> auto eval(Ctx& ctx, BinExpr<L, R, tag::_add>)    { return eval(ctx, L{}) + eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto eval(Ctx& ctx, BinExpr<L, R, tag::_sub>)    { return eval(ctx, L{}) - eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto eval(Ctx& ctx, BinExpr<L, R, tag::_mul>)    { return eval(ctx, L{}) * eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto eval(Ctx& ctx, BinExpr<L, R, tag::_div>)    { return eval(ctx, L{}) / eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto eval(Ctx& ctx, BinExpr<L, R, tag::_exp>)    { return pow(eval(ctx, L{}), eval(ctx, R{})); }
    template <typename L, typename R, typename Ctx> auto eval(Ctx& ctx, BinExpr<L, R, tag::_assign>) { return eval(ctx, L{}) = eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto&& eval(Ctx& ctx, BinExpr<L, R, tag::_add_assign>) { return eval(ctx, L{}) += eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto&& eval(Ctx& ctx, BinExpr<L, R, tag::_sub_assign>) { return eval(ctx, L{}) -= eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto&& eval(Ctx& ctx, BinExpr<L, R, tag::_mul_assign>) { return eval(ctx, L{}) *= eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto&& eval(Ctx& ctx, BinExpr<L, R, tag::_div_assign>) { return eval(ctx, L{}) /= eval(ctx, R{}); }
    template <typename L, typename R, typename Ctx> auto&& eval(Ctx& ctx, BinExpr<L, R, tag::_exp_assign>) { return eval(ctx, L{}) = pow(eval(ctx, L{}), eval(ctx, R{})); }
}
