/**
 *
 * This file is part of the libvcard project.
 *
 * Copyright (C) 2010, Emanuele Bertoldi (Card Tech srl).
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Revision: 10 $
 * $Date: 2010-10-18 10:50:23 +0200 (Mon, 18 Oct 2010) $
 */

#ifndef VCARDPROPERTY_H
#define VCARDPROPERTY_H

#include "vcardparam.h"
#include <QDateTime>
#include <QStringList>

#define VC_ADDRESS              "ADR"
#define VC_AGENT                "AGENT"
#define VC_BIRTHDAY             "BDAY"
#define VC_CATEGORIES           "CATEGORIES"
#define VC_CLASS                "CLASS"
#define VC_DELIVERY_LABEL       "LABEL"
#define VC_EMAIL                "EMAIL"
#define VC_FORMATTED_NAME       "FN"
#define VC_GEOGRAPHIC_POSITION  "GEO"
#define VC_KEY                  "KEY"
#define VC_LOGO                 "LOGO"
#define VC_MAILER               "MAILER"
#define VC_NAME                 "N"
#define VC_NICKNAME             "NICKNAME"
#define VC_NOTE                 "NOTE"
#define VC_ORGANIZATION         "ORG"
#define VC_PHOTO                "PHOTO"
#define VC_PRODUCT_IDENTIFIER   "PRODID"
#define VC_REVISION             "REV"
#define VC_ROLE                 "ROLE"
#define VC_SORT_STRING          "SORT-STRING"
#define VC_SOUND                "SOUND"
#define VC_TELEPHONE            "TEL"
#define VC_TIME_ZONE            "TZ"
#define VC_TITLE                "TITLE"
#define VC_URL                  "URL"
#define VC_VERSION              "VERSION"
#define VC_KHID                 "X-KEYHOTEE-ID"
#define VC_KH_PUBLIC_KEY        "X-KEYHOTEE-PublicKey"

class vCardProperty
{
public:
    enum GenericFields
    {
        DefaultValue = 0
    };

    enum AddressFields
    {
        PostOfficeBox = 0,
        ExtendedAddress,
        Street,
        Locality,           // e.g. City.
        Region,             // e.g. State or province.
        PostalCode,
        Country
    };

    enum NameFields
    {
        Lastname = 0,
        Firstname,
        Additional,
        Prefix,
        Suffix
    };

    enum GeographicPositionFields
    {
        Latitude = 0,
        Longitude
    };

public:
    vCardProperty();
    vCardProperty(const QString& name, const QString& value, const vCardParamList& params = vCardParamList());
    vCardProperty(const QString& name, const QStringList& values, const vCardParamList& params = vCardParamList());
    vCardProperty(const QString& name, const QString& value, const QString& params);
    vCardProperty(const QString& name, const QStringList& values, const QString& params);
    ~vCardProperty();

    QString name() const;
    QString value() const;
    QStringList values() const;
    vCardParamList params() const;
    bool isValid() const;

    bool operator== (const vCardProperty& param) const;
    bool operator!= (const vCardProperty& param) const;

    QByteArray toByteArray(vCardVersion version = VC_VER_2_1) const;

    static QList<vCardProperty> fromByteArray(const QByteArray& data);

    static vCardProperty createAddress(const QString& street, const QString& locality, const QString& region, const QString& postal_code, const QString& country, const QString& post_office_box = "", const QString& ext_address = "", const vCardParamList& params = vCardParamList());
    static vCardProperty createBirthday(const QDate& birthday, const vCardParamList& params = vCardParamList());
    static vCardProperty createBirthday(const QDateTime& birthday, const vCardParamList& params = vCardParamList());
    static vCardProperty createGeographicPosition(qreal latitude, qreal longitude, const vCardParamList& params = vCardParamList());
    static vCardProperty createName(const QString& firstname, const QString& lastname, const QString& additional = "", const QString& prefix = "", const QString& suffix = "", const vCardParamList& params = vCardParamList());
    static vCardProperty createdFormattedName(const QString& name);
    static vCardProperty createOrganization(const QString& name, const QStringList& levels = QStringList(), const vCardParamList& params = vCardParamList());
    static vCardProperty createKHID(const QString& khID, const vCardParamList& params = vCardParamList());
    static vCardProperty createPublicKey(const QString& publicKey, const vCardParamList& params = vCardParamList());
    static vCardProperty createNotes(const QString& name);
    static vCardProperty createAvatar(const std::vector<char>& iconData);

protected:
    QString m_name;
    QStringList m_values;
    vCardParamList m_params;
};

typedef QList<vCardProperty> vCardPropertyList;

#endif // VCARDPROPERTY_H
