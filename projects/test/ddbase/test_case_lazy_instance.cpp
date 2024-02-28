#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddlazy_instance.h"

namespace NSP_DD {

class A
{
public:
    A()
    {

    }

    ~A()
    {
    }

    void Test()
    {

    }
};

class B
{
public:
    B(int, int)
    {
    }
    ~B()
    {
    }
    void Test()
    {

    }
};

class C : public A, public B
{
public:
    C(int a, int b) : B(a, b)
    {
    }
    ~C()
    {

    }
    void Test()
    {

    }
};;

DDTEST(test_ddlazy_instance, test1)
{
    {
        A a;
        B b(1, 2);
        C c(1, 2);
        DDLAZY_INSTANCE_REGISTER(&a);
        DDLAZY_INSTANCE_REGISTER(&b);
        DDLAZY_INSTANCE_REGISTER(&c);
        DDLAZY_INSTANCE(A).Test();
        DDLAZY_INSTANCE(B).Test();
        DDLAZY_INSTANCE(C).Test();
    }
}

} // namespace NSP_DD
