#pragma once
#include "Interfaces/Command.h"
#include "API/API.h"

namespace Commands
{
    // One engine bool setting: {"request": name} reads it,
    // {"request": name, "value": bool} sets it. Replies {"req": name, "val": bool}.
    class BoolSettingHandler : public Interface::CommandHandler
    {
    public:
        using Member = bool Config::Engine::*;
        using Setter = void (API::*)(bool, Origin);

        BoolSettingHandler(API& api, const char* name, Member member, Setter setter);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        API& m_api;
        const char* m_name;
        Member m_member;
        Setter m_setter;
    };
}
