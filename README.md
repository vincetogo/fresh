Useful utility classes

signal - templates which allows you to add the observer pattern to your designs. Its interface should be familiar to anyone who has used boost::signals2

property - templates for high-level style accessors and mutators, which adopt the observer pattern using signal to notify interested objects that they've changed. (Signal-less 'light' versions of the properties are available when the observer pattern isn't needed).
