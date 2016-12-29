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
            f4(this)
        {
            
        }
        
        ~A()
        {
        }
        
        void func()
        {
            // we can set f2 from within A
            f2 = 4.0f;
        }

        property<float, light>          f1 = 3;
        property<float, writable_by<A>> f2 = 3;
        property<int, read_only>        i1 = 14;
        
        float&
        get_f3()
        {
            return _f3;
        }
        
        property<float, dynamic<A>, &A::get_f3> f3;

        float&
        get_f4()
        {
            return _f4;
        }
        
        void
        set_f4(const float& other)
        {
            _f4 = other;
            f4.send();
        }
        
        property<float, dynamic<A>, &A::get_f4, &A::set_f4> f4;
        property<float, writable<assign_different>>         f5 = 3.0f;
        
        property<std::shared_ptr<A>>                        another_a;
        
    private:
        float _f3 = 1.0f;
        float _f4 = 2.0f;
    };
}

int main(int argc, const char * argv[]) {
    
    A a;
    
    a.f1 = 4.0f;
    //a.f2 = 4.0f; -> Error: f2 is only writable by A
    //a.i1 = 18; -> Error: i1 is read_only
    //a.f3 = 5.0f; -> Error: f3 doesn't have a setter
    a.f4 = 5.0f;
    
    a.f4.connect(
        [&]()
        {
            printf("f4 is now %s\n", std::to_string(a.f4()).c_str());
        });
    
    a.f4 = 6.0f;
    
    a.f5.connect(
        [&]()
        {
            printf("f5 is now %s\n", std::to_string(a.f5()).c_str());
        });
    
    a.f5 = 6.0f;
    a.f5 = 6.0f; // f5 is assign_different and doesn't change here so no signal is sent
    a.f5 = 3.0f;
    
    a.f5 += 1.6f;
    
    a.f5 ++;
    
    a.f5 -= 3.1f;
    a.f5 --;
    
    a.another_a = std::make_shared<A>();
    
    return 0;
}
