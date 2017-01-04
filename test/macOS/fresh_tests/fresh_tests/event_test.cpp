//
// connection.cpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#include "event.hpp"

void event_test()
{
    fresh::event<void()> e;
    
    std::vector<fresh::ev_connection> cnxns;
    
    auto cnxn = e.connect(
        []() -> void
        {
            //return 3;
        });
    
    cnxns.push_back(std::move(cnxn));
    //cnxns.clear();
    
    e();
}
