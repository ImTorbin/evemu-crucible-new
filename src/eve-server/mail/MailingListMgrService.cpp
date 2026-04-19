/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Luck, caytchen
*/

#include "eve-server.h"

#include "EVE_Mail.h"
#include "mail/MailingListMgrService.h"

namespace {

bool PyListToCharacterIDs(PyList* members, std::vector<int32>& memberIDs, const char* methodName)
{
    memberIDs.clear();
    memberIDs.reserve(members->size());

    PyList::const_iterator cur = members->begin();
    for (size_t index = 0; cur != members->end(); ++cur, ++index) {
        if (!(*cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode %s failed: element %u is not an integer: %s", methodName, index, (*cur)->TypeString());
            return false;
        }

        memberIDs.push_back((*cur)->AsInt()->value());
    }

    return true;
}

bool CallerHasMailingListRole(MailDB& db, Client* client, int32 listID, bool allowOperators)
{
    int32 role = mailingListMemberDefault;
    if (!db.GetMailingListRole(client->GetCharacterID(), listID, role))
        return false;

    if (role == mailingListMemberOwner)
        return true;

    return allowOperators && (role == mailingListMemberOperator);
}

}

MailingListMgrService::MailingListMgrService() :
    Service("mailingListsMgr", eAccessLevel_Character)
{
    this->Add("GetJoinedLists", &MailingListMgrService::GetJoinedLists);
    this->Add("Create", &MailingListMgrService::Create);
    this->Add("Join", &MailingListMgrService::Join);
    this->Add("Leave", &MailingListMgrService::Leave);
    this->Add("Delete", &MailingListMgrService::Delete);
    this->Add("KickMembers", &MailingListMgrService::KickMembers);
    this->Add("GetMembers", &MailingListMgrService::GetMembers);
    this->Add("SetEntityAccess", &MailingListMgrService::SetEntityAccess);
    this->Add("ClearEntityAccess", &MailingListMgrService::ClearEntityAccess);
    this->Add("SetMembersMuted", &MailingListMgrService::SetMembersMuted);
    this->Add("SetMembersOperator", &MailingListMgrService::SetMembersOperator);
    this->Add("SetMembersClear", &MailingListMgrService::SetMembersClear);
    this->Add("SetDefaultAccess", &MailingListMgrService::SetDefaultAccess);
    this->Add("GetInfo", &MailingListMgrService::GetInfo);
    this->Add("GetSettings", &MailingListMgrService::GetSettings);
    this->Add("GetWelcomeMail", &MailingListMgrService::GetWelcomeMail);
    this->Add("SaveWelcomeMail", &MailingListMgrService::SaveWelcomeMail);
    this->Add("SendWelcomeMail", &MailingListMgrService::SendWelcomeMail);
    this->Add("ClearWelcomeMail", &MailingListMgrService::ClearWelcomeMail);
}

PyResult MailingListMgrService::GetJoinedLists(PyCallArgs& call)
{
    // @TODO: Test
    // no args
    sLog.Debug("MailingListMgrService", "Called GetJoinedLists stub" );

    return m_db.GetJoinedMailingLists(call.client->GetCharacterID());
}

PyResult MailingListMgrService::Create(PyCallArgs& call, PyWString* name, PyInt* defaultAccess, PyInt* defaultMemberAccess, std::optional<PyInt*> mailCost)
{
    sLog.Debug("MailingListMgrService", "Called Create stub" );
    uint32 r = m_db.CreateMailingList(call.client->GetCharacterID(), name->content(), defaultAccess->value(),
                                   defaultMemberAccess->value(), mailCost.has_value() ? mailCost.value()->value() : 0);
    if (r != static_cast<uint32>(-1)) {
        return new PyInt(r);
    }
    return nullptr;
}

PyResult MailingListMgrService::Join(PyCallArgs& call, PyRep* listName)
{
    sLog.Debug("MailingListMgrService", "Called Join stub" );

    std::string listNameStr = PyRep::StringContent (listName);
    uint32 listID = 0;
    if (!m_db.JoinMailingList(call.client->GetCharacterID(), listNameStr, listID))
        return nullptr;

    return m_db.GetMailingListInfo(call.client->GetCharacterID(), listID);
}

PyResult MailingListMgrService::Leave(PyCallArgs& call, PyInt* listID)
{
    sLog.Debug("MailingListMgrService", "Called Leave stub" );
    m_db.LeaveMailingList(call.client->GetCharacterID(), listID->value());
    return nullptr;
}

PyResult MailingListMgrService::Delete(PyCallArgs& call, PyInt* listID)
{
    sLog.Debug("MailingListMgrService", "Called Delete stub" );
    if (!CallerHasMailingListRole(m_db, call.client, listID->value(), false))
        return nullptr;

    m_db.DeleteMailingList(call.client->GetCharacterID(), listID->value());
    return nullptr;
}

PyResult MailingListMgrService::KickMembers(PyCallArgs& call, PyInt* listID, PyList* memberIDs)
{
    sLog.Debug("MailingListMgrService", "Called KickMembers stub" );
    if (!CallerHasMailingListRole(m_db, call.client, listID->value(), true))
        return nullptr;

    std::vector<int32> members;
    if (!PyListToCharacterIDs(memberIDs, members, "MailingListMgrService::KickMembers"))
        return nullptr;

    m_db.KickMailingListMembers(listID->value(), members);
    return nullptr;
}

PyResult MailingListMgrService::GetMembers(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called GetMembers stub" );
    int32 role = mailingListMemberDefault;
    if (!m_db.GetMailingListRole(call.client->GetCharacterID(), listID->value(), role))
        return nullptr;

    return m_db.GetMailingListMembers(listID->value());
}

PyResult MailingListMgrService::SetEntityAccess(PyCallArgs& call, PyInt* listID, PyInt* entityID, PyInt* access)
{
    sLog.Debug("MailingListMgrService", "Called SetEntityAccess stub" );
    if (!CallerHasMailingListRole(m_db, call.client, listID->value(), true))
        return nullptr;

    m_db.MailingListSetEntityAccess(entityID->value(), access->value(), listID->value());
    return nullptr;
}

PyResult MailingListMgrService::ClearEntityAccess(PyCallArgs& call, PyInt* listID, PyInt* entityID)
{
    sLog.Debug("MailingListMgrService", "Called ClearEntityAccess stub" );
    if (!CallerHasMailingListRole(m_db, call.client, listID->value(), true))
        return nullptr;

    m_db.MailingListClearEntityAccess(entityID->value(), listID->value());
    return nullptr;
}

PyResult MailingListMgrService::SetMembersMuted(PyCallArgs& call, PyInt* listID, PyList* memberIDs)
{
    sLog.Debug("MailingListMgrService", "Called SetMembersMuted stub" );
    if (!CallerHasMailingListRole(m_db, call.client, listID->value(), true))
        return nullptr;

    std::vector<int32> members;
    if (!PyListToCharacterIDs(memberIDs, members, "MailingListMgrService::SetMembersMuted"))
        return nullptr;

    m_db.SetMailingListMembersRole(listID->value(), members, mailingListMemberMuted);
    return nullptr;
}

PyResult MailingListMgrService::SetMembersOperator(PyCallArgs& call, PyInt* listID, PyList* memberIDs)
{
    sLog.Debug("MailingListMgrService", "Called SetMembersOperator stub" );
    if (!CallerHasMailingListRole(m_db, call.client, listID->value(), true))
        return nullptr;

    std::vector<int32> members;
    if (!PyListToCharacterIDs(memberIDs, members, "MailingListMgrService::SetMembersOperator"))
        return nullptr;

    m_db.SetMailingListMembersRole(listID->value(), members, mailingListMemberOperator);
    return nullptr;
}

PyResult MailingListMgrService::SetMembersClear(PyCallArgs& call, PyInt* listID, PyList* memberIDs)
{
    sLog.Debug("MailingListMgrService", "Called SetMembersClear stub" );
    if (!CallerHasMailingListRole(m_db, call.client, listID->value(), true))
        return nullptr;

    std::vector<int32> members;
    if (!PyListToCharacterIDs(memberIDs, members, "MailingListMgrService::SetMembersClear"))
        return nullptr;

    m_db.SetMailingListMembersRole(listID->value(), members, mailingListMemberDefault);
    return nullptr;
}

PyResult MailingListMgrService::SetDefaultAccess(PyCallArgs& call, PyInt* listID, PyInt* defaultAccess, PyInt* defaultMemberAccess, std::optional<PyInt*> mailCost)
{
    sLog.Debug("MailingListMgrService", "Called SetDefaultAccess stub" );
    if (!CallerHasMailingListRole(m_db, call.client, listID->value(), false))
        return nullptr;

    m_db.SetMailingListDefaultAccess(listID->value(), defaultAccess->value(),
                                      defaultMemberAccess->value(), mailCost.has_value() ? mailCost.value()->value() : 0);

    return nullptr;
}

PyResult MailingListMgrService::GetInfo(PyCallArgs& call, PyInt* listID)
{
    sLog.Debug("MailingListMgrService", "Called GetInfo stub" );
    return m_db.GetMailingListInfo(call.client->GetCharacterID(), listID->value());
}

PyResult MailingListMgrService::GetSettings(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Test
    sLog.Debug("MailingListMgrService", "Called GetSettings stub" );
    int32 role = mailingListMemberDefault;
    if (!m_db.GetMailingListRole(call.client->GetCharacterID(), listID->value(), role))
        return nullptr;

    // return:
    // .access: list (ownerID, accessLevel)
    // .defaultAccess
    // .defaultMemberAccess

    return m_db.MailingListGetSettings(listID->value());
}

PyResult MailingListMgrService::GetWelcomeMail(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called GetWelcomeMail stub" );
    return nullptr;
}

PyResult MailingListMgrService::SaveWelcomeMail(PyCallArgs& call, PyInt* listID, PyWString* title, PyWString* body)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SaveWelcomeMail stub" );

    return nullptr;
}

PyResult MailingListMgrService::SendWelcomeMail(PyCallArgs& call, PyInt* listID, PyWString* title, PyWString* body)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SendWelcomeMail stub" );
    return nullptr;
}

PyResult MailingListMgrService::ClearWelcomeMail(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called ClearWelcomeMail stub" );

    // no return value
    return nullptr;
}
