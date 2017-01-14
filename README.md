Useful utility classes

event - templated class which allow you to easily add the observer pattern to your designs

property - templated class for high-level style accessors and mutators, and come in a variety of flavours - a basic field version, a read-only field, a field that's writable only by a specified class (useful for properties that should be accessible by other classes but only writable by the class that owns the property), and dynamic versions, which can either have just a getter function or a getter and setter. Properties can also be thread safe and/or observable using the event class mentioned above
