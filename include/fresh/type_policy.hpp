//
//  type_policy.hpp
//  Prog
//
//  Created by Vince Tourangeau on 1/29/17.
//  Copyright © 2017 Cryogenic Head. All rights reserved.
//

#ifndef fresh_type_policy_h
#define fresh_type_policy_h

enum class type_policy
{
    copy,
    reference
};

const auto copy = type_policy::copy;
const auto reference = type_policy::reference;

#endif /* type_policy_h */
