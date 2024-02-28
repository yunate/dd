#ifndef ddbase_macro_ddcat_hpp_
#define ddbase_macro_ddcat_hpp_

// string
#define _DDTOSTRING(x) #x
#define DDTOSTRING(x) _DDTOSTRING(x)

#define _DDCAT(a, b) a ## b
#define DDCAT(a, b) _DDCAT(a, b)

#define DDEXPEND(...) __VA_ARGS__
#endif // ddbase_macro_ddcat_hpp_
