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
    
    std::vector<fresh::connection> cnxns;
    
    auto cnxn = e.connect(
        [&]() -> void
        {
            cnxns.clear();
        });
    
    cnxns.push_back(std::move(cnxn));
    
    e();
}
