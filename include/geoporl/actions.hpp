#pragma once
#include <string>
#include <vector>

enum class Action : int {
    IncreaseDefenseSpending = 0,
    IncreaseEducationFunding,
    LowerTaxes,
    RaiseTaxes,
    InvestInInfrastructure,
    IncreaseWelfare,
    InvestInRenewables,
    ImproveCyberSecurity,
    StrengthenAlliances,
    LimitImmigration,
    OpenImmigration,

    ACTION_COUNT // must be last entry
};

inline std::string action_name(Action a) {
    switch(a) {
        case Action::IncreaseDefenseSpending: return "IncreaseDefenseSpending";
        case Action::IncreaseEducationFunding: return "IncreaseEducationFunding";
        case Action::LowerTaxes: return "LowerTaxes";
        case Action::RaiseTaxes: return "RaiseTaxes";
        case Action::InvestInInfrastructure: return "InvestInInfrastructure";
        case Action::IncreaseWelfare: return "IncreaseWelfare";
        case Action::InvestInRenewables: return "InvestInRenewables";
        case Action::ImproveCyberSecurity: return "ImproveCyberSecurity";
        case Action::StrengthenAlliances: return "StrengthenAlliances";
        case Action::LimitImmigration: return "LimitImmigration";
        case Action::OpenImmigration: return "OpenImmigration";
        default: return "Unknown";
    }
}