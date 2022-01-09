//
// C++ Implementation: starname
//
// Description:
//
//
// Author: Toti <root@totibox>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <fmt/format.h>
#include <celengine/constellation.h>
#include <celengine/starname.h>
#include <celutil/greek.h>

using namespace std;


uint32_t StarNameDatabase::findCatalogNumberByName(const string& name, bool i18n) const
{
    auto catalogNumber = getCatalogNumberByName(name, i18n);
    if (catalogNumber != AstroCatalog::InvalidIndex)
        return catalogNumber;

    string priName(name);
    string altName;

    bool isOrbitingStar = false; // star is an orbiting one if it looks like `Barycenter A`
    // See if the name is a Bayer or Flamsteed designation
    auto pos = name.find(' ');
    auto nameLength = name.length();
    if (pos != 0 && pos != string::npos && pos < nameLength - 1)
    {
        auto pos1 = name.rfind(' ');
        std::string_view rest;
        if (pos1 != string::npos && pos1 > pos && pos1 < nameLength - 1 && isalpha(name[pos1 + 1]))
        {
            rest = std::string_view(name).substr(pos1);
            isOrbitingStar = true;
            do { --pos1; } while (isspace(name[pos1]));
        }
        string prefix(name, 0, pos);
        string conName(name, pos + 1, isOrbitingStar ? pos1 - pos : string::npos);

        Constellation* con  = Constellation::getConstellation(conName);
        if (con != nullptr)
        {
            char digit  = ' ';
            int len = prefix.length();

            // If the first character of the prefix is a letter
            // and the last character is a digit, we may have
            // something like 'Alpha2 Cen' . . . Extract the digit
            // before trying to match a Greek letter.
            if (len > 2 && isalpha(prefix[0]) && isdigit(prefix[len - 1]))
            {
                --len;
                digit = prefix[len];
            }

            // We have a valid constellation as the last part
            // of the name.  Next, we see if the first part of
            // the name is a greek letter.
            std::string_view letter = GetCanonicalGreekAbbreviation(std::string_view(prefix).substr(0, len));
            if (!letter.empty())
            {
                // Matched . . . this is a Bayer designation
                if (digit == ' ')
                {
                    priName  = fmt::format("{} {}", letter, con->getAbbreviation());
                    // If 'let con' doesn't match, try using
                    // 'let1 con' instead.
                    altName  = fmt::format("{}1 {}", letter, con->getAbbreviation());
                }
                else
                {
                    priName = fmt::format("{}{} {}", letter, digit, con->getAbbreviation());
                }
            }
            else
            {
                // Something other than a Bayer designation
                priName = fmt::format("{} {}", prefix, con->getAbbreviation());
            }

            if (isOrbitingStar)
            {
                priName.append(rest);
                if (!altName.empty())
                    altName.append(rest);
            }
        }

        catalogNumber = getCatalogNumberByName(priName, i18n);
        if (catalogNumber != AstroCatalog::InvalidIndex)
            return catalogNumber;
    }

    if (!isOrbitingStar)
    {
        priName.append(" A");  // try by appending an A
        catalogNumber = getCatalogNumberByName(priName, i18n);
        if (catalogNumber != AstroCatalog::InvalidIndex)
            return catalogNumber;
    }

    // If the first search failed, try using the alternate name
    if (!altName.empty())
    {
        catalogNumber = getCatalogNumberByName(altName, i18n);
        if (catalogNumber == AstroCatalog::InvalidIndex && !isOrbitingStar)
        {
            altName.append(" A");
            catalogNumber = getCatalogNumberByName(altName, i18n);
        }   // Intentional fallthrough.
    }

    return catalogNumber;
}


StarNameDatabase* StarNameDatabase::readNames(istream& in)
{
    StarNameDatabase* db = new StarNameDatabase();
    bool failed = false;
    string s;

    while (!failed)
    {
        auto catalogNumber = AstroCatalog::InvalidIndex;

        in >> catalogNumber;
        if (in.eof())
            break;
        if (in.bad())
        {
            failed = true;
            break;
        }

        // in.get(); // skip a space (or colon);

        string name;
        getline(in, name);
        if (in.bad())
        {
            failed = true;
            break;
        }

        // Iterate through the string for names delimited
        // by ':', and insert them into the star database. Note that
        // db->add() will skip empty names.
        string::size_type startPos = 0;
        while (startPos != string::npos)
        {
            ++startPos;
            string::size_type next = name.find(':', startPos);
            string::size_type length = string::npos;

            if (next != string::npos)
                length = next - startPos;

            db->add(catalogNumber, name.substr(startPos, length));
            startPos = next;
        }
    }

    if (failed)
    {
        delete db;
        return nullptr;
    }
    else
    {
        return db;
    }
}
