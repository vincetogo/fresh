//
//  main.cpp
//  fresh_tests
//
//  Created by Vince on 2016-12-29.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#include <iostream>

#include <fresh/property.hpp>

#include <memory>

namespace
{
    using namespace fresh;
    
    class A
    {
    public:
        A() :
            f3(this),
            f5(this, f3, f4)
        {
        }
        
        ~A()
        {
        }
        
        property<std::shared_ptr<A>>                            another_a;
        
        property<float, light>          f1 = 3;
        property<float, writable_by<A>> f2 = 3;
        property<int, read_only>        i1 = 14;
        
        const float&
        get_f3() const
        {
            return _f3;
        }
        
        void
        set_f3(const float& other)
        {
            _f3 = other;
            f3.send();
        }
        
        property<float&, dynamic<A>, &A::get_f3, &A::set_f3>    f3;
        property<float, writable<assign_different>>             f4 = 3.0f;
        
        float
        get_f5() const
        {
            return f3() + f4();
        }
        
        property<float, dynamic<A>, &A::get_f5>                 f5;
        
        void func()
        {
            // we can set f2 from within A
            f2 = 4.0f;
        }

    private:
        float _f3 = 2.0f;
    };
}

int main(int argc, const char * argv[])
{
    A a;
    
    a.f1 = 4.0f;
    //a.f2 = 4.0f; -> Error: f2 is only writable by A
    //a.i1 = 18; -> Error: i1 is read_only
    //a.f3 = 5.0f; -> Error: f3 doesn't have a setter
    a.f4 = 5.0f;
    
    a.f3.connect(
        [&]()
        {
            printf("f4 is now %f\n", a.f3());
        });
    
    a.f4.connect(
        [&]()
        {
            printf("f4 is now %f\n", a.f4());
        });
    
    auto cnxn = a.f5.connect(
        [&]()
        {
            printf("f5 is now %f\n", a.f5());
        });
    
    {
        a.f3 = 6.0f;
        
        a.f4 = 6.0f;
        a.f4 = 6.0f; // f4 is assign_different and doesn't change here so no signal is sent
        a.f4 = 3.0f;
        a.f4 = 4.0f;
    }
    
    a.f4 += 1.6f;
    
    a.f4 ++;
    
    a.f4 -= 3.15f;
    a.f4 --;
    
    //a.another_a = std::make_shared<A>();
    
    printf("f3: %f\n", a.f3());
    
    return 0;
}
