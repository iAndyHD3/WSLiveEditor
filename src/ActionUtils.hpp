#define GLZ_ACTION_META(Type)                                                                        \
    template <>                                                                                      \
    struct glz::meta<Type>                                                                           \
    {                                                                                                \
        using T = Type;                                                                              \
                                                                                                     \
        /* The constraint check: ensures the deserialized 'action' string matches the expected constant. */ \
        static constexpr auto limit_action = [](const T&, const std::string& action) {                \
            return action == T::ACTION_NAME;                                                         \
        };                                                                                           \
                                                                                                     \
        /* Defines the object structure, applying the constraint to the 'action' field. */             \
        static constexpr auto modify = glz::object(                                                   \
            "action", glz::read_constraint<&T::action, limit_action, "Action does not match">        \
        );                                                                                           \
    };


#define CHECK_ACTION(Type) \
if(auto runner = glz::read_json<Type>(msg); runner.has_value()) \
{ \
    if(Type::EDITOR_ACTION && !g_inEditor) {\
       channel->send("{\"status\":\"error\",\"error\":\"Enter the level editor to run this action\"}");\
       return;\
    }\
    std::lock_guard lock(g_actionsMutex);\
    g_actions.emplace_back(channel, pro::make_proxy<Runnable>(*runner), runner->close); \
}        