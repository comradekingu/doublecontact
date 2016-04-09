/* Double Contact
 *
 * Module: Data and container structures
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include "contactlist.h"
#include <QMessageBox>

Phone::StandardTypes::StandardTypes Phone::standardTypes;

Phone::StandardTypes::StandardTypes()
{
    clear();
    (*this)["home"] = QObject::tr("Home");
    (*this)["msg"] = QObject::tr("Msg");
    (*this)["work"] = QObject::tr("Work");
    (*this)["pref"] = QObject::tr("Pref");
    (*this)["voice"] = QObject::tr("Voice"); // Synonym for OTHER for some real phones
    (*this)["fax"] = QObject::tr("Fax");
    (*this)["cell"] = QObject::tr("Cell");
    (*this)["video"] = QObject::tr("Video");
    (*this)["pager"] = QObject::tr("Pager");
    (*this)["bbs"] = QObject::tr("BBS");
    (*this)["modem"] = QObject::tr("Modem");
    (*this)["car"] = QObject::tr("Car");
    (*this)["isdn"] = QObject::tr("ISDN");
    (*this)["pcs"] = QObject::tr("PCS");
}

void Phone::calculateFields()
{
    isMixed = tTypes.count()>1;
}

void ContactItem::clear()
{
    fullName.clear();
    names.clear();
    phones.clear();
    emails.clear();
    birthDay = QDateTime();
    description.clear();
    unknownTags.clear();
    originalFormat.clear();
    version.clear();
    // maybe not needed:
    prefPhone.clear();
    prefEmail.clear();
}

bool ContactItem::swapNames()
{
    if (names.isEmpty())
        return false;
    if (names.count()==1) names.push_back("");
    QString buffer = names[0];
    names[0] = names[1];
    names[1] = buffer;
    return true;
}

void ContactItem::calculateFields()
{
    prefPhone.clear(); // first or preferred phone number
    if (phones.count()>0) {
        prefPhone = phones[0].number;
        for (int i=0; i<phones.count();i++) {
            if (phones[i].tTypes.contains("pref"))
                prefPhone = phones[i].number;
            phones[i].calculateFields();
        }
    }
    prefEmail.clear(); // first or preferred email
    if (emails.count()>0) {
        prefEmail = emails[0].address;
        for (int i=0; i<emails.count(); i++)
            if (emails[i].preferred) // TODO preferred, вероятно, убрать, у почты тоже есть тег pref
                prefEmail = emails[i].address;
    }
}

ContactList::ContactList()
{
}


TagValue::TagValue(const QString& _tag, const QString& _value)
    :tag(_tag), value(_value)
{}

