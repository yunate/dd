
#ifndef ddbase_ddnocopyable_hpp_
#define ddbase_ddnocopyable_hpp_

#define DDNO_COPY(className)                                    \
    className(const className&) = delete;                       \
    className& operator= (const className&) = delete;           \

#define DDNO_MOVE(className)                                    \
    className(className&&) = delete;                            \
    className& operator= (className&&) = delete;                \

#define DDNO_COPY_MOVE(className)                              \
    DDNO_COPY(className);                                      \
    DDNO_MOVE(className);                                      \

#define DDDEFAULT_COPY(className)                              \
    className(const className&) = default;                     \
    className& operator= (const className&) = default;         \

#define DDDEFAULT_MOVE(className)                              \
    className(className&&) = default;                          \
    className& operator= (className&&) = default;              \

#define DDDEFAULT_COPY_MOVE(className)                         \
    DDDEFAULT_COPY(className);                                 \
    DDDEFAULT_MOVE(className);                                 \

#define DDDEFAULT_CONSTRUCT_ALL(className)                     \
    DDDEFAULT_COPY_MOVE(className)                             \
    className() = default;                                     \
    ~className() = default;                                    \

#endif // ddbase_ddnocopyable_hpp_
