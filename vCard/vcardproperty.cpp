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
 * $Revision: 12 $
 * $Date: 2011-02-20 15:00:41 +0100 (Sun, 20 Feb 2011) $
 */

#include "vcardproperty.h"

#define VC_ASSIGNMENT_TOKEN ':'

vCardProperty::vCardProperty()
{
}

vCardProperty::vCardProperty(const QString& name, const QString& value, const vCardParamList& params)
    :   m_name(name),
        m_params(params)
{
    m_values = value.split(VC_SEPARATOR_TOKEN);
}

vCardProperty::vCardProperty(const QString& name, const QStringList& values, const vCardParamList& params)
    :   m_name(name),
        m_values(values),
        m_params(params)
{
}

vCardProperty::vCardProperty(const QString& name, const QString& value, const QString& params)
    :   m_name(name)
{
    m_values = value.split(VC_SEPARATOR_TOKEN);
    m_params = vCardParam::fromByteArray(params.toUtf8());
}

vCardProperty::vCardProperty(const QString& name, const QStringList& values, const QString& params)
    :   m_name(name),
        m_values(values)
{
    m_params = vCardParam::fromByteArray(params.toUtf8());
}

vCardProperty::~vCardProperty()
{
}

QString vCardProperty::name() const
{
    return m_name;
}

QString vCardProperty::value() const
{
    return m_values.join(QString(VC_SEPARATOR_TOKEN));
}

QStringList vCardProperty::values() const
{
    return m_values;
}

vCardParamList vCardProperty::params() const
{
    return m_params;
}

bool vCardProperty::isValid() const
{
    if (m_name.isEmpty())
        return false;

    if (m_values.isEmpty())
        return false;

    for (const vCardParam& param : m_params)
        if (!param.isValid())
            return false;

    return true;
}

bool vCardProperty::operator== (const vCardProperty& prop) const
{
    return ((m_name == prop.name()) && (m_values == prop.values()));
}

bool vCardProperty::operator!= (const vCardProperty& prop) const
{
    return ((m_name != prop.name()) || (m_values != prop.values()));
}

QByteArray vCardProperty::toByteArray(vCardVersion version) const
{
    QByteArray buffer;

    switch (version)
    {
        case VC_VER_2_1:
        case VC_VER_3_0:
        {
            buffer.append(m_name).toUpper();
            QByteArray params = vCardParam::toByteArray(m_params, version);

            if (!params.isEmpty())
            {
                buffer.append(VC_SEPARATOR_TOKEN);
                buffer.append(params);
            }

            buffer.append(QString(VC_ASSIGNMENT_TOKEN));
            buffer.append(m_values.join(QString(VC_SEPARATOR_TOKEN)));
        }
        break;

        default:
            break;
    }

    return buffer;
}

QList<vCardProperty> vCardProperty::fromByteArray(const QByteArray& data)
{
    QList<vCardProperty> properties;

    QStringList lines = QString::fromUtf8(data).split(VC_END_LINE_TOKEN);
    for (const QString& line : lines)
    {
        if (line == VC_BEGIN_TOKEN || line == VC_END_TOKEN)
            break;

        QStringList tokens = line.split(VC_ASSIGNMENT_TOKEN);
        if (tokens.count() >= 2)
        {
            QStringList property_tokens = tokens.at(0).split(VC_SEPARATOR_TOKEN);
            QString name = property_tokens.takeAt(0);

            if (name != VC_VERSION)
            {
                vCardParamList params = vCardParam::fromByteArray(property_tokens.join(QString(VC_SEPARATOR_TOKEN)).toUtf8());

                properties.append(vCardProperty(name, tokens.at(1), params));
            }
        }
    }

    return properties;
}

vCardProperty vCardProperty::createAddress(const QString& street, const QString& locality, const QString& region, const QString& postal_code, const QString& country, const QString& post_office_box, const QString& ext_address, const vCardParamList& params)
{
    QStringList values;
    values.insert(vCardProperty::PostOfficeBox, post_office_box);
    values.insert(vCardProperty::ExtendedAddress, ext_address);
    values.insert(vCardProperty::Street, street);
    values.insert(vCardProperty::Locality, locality);
    values.insert(vCardProperty::Region, region);
    values.insert(vCardProperty::PostalCode, postal_code);
    values.insert(vCardProperty::Country, country);

    return vCardProperty(VC_ADDRESS, values, params);
}

vCardProperty vCardProperty::createBirthday(const QDate& birthday, const vCardParamList& params)
{
    return vCardProperty(VC_BIRTHDAY, birthday.toString("yyyy-MM-dd"), params);
}

vCardProperty vCardProperty::createBirthday(const QDateTime& birthday, const vCardParamList& params)
{
    return vCardProperty(VC_BIRTHDAY, birthday.toString("yyyy-MM-ddThh:mm:ssZ"), params);
}

vCardProperty vCardProperty::createGeographicPosition(qreal latitude, qreal longitude, const vCardParamList& params)
{
    QStringList values;
    values.insert(vCardProperty::Latitude, QString("%1").arg(latitude));
    values.insert(vCardProperty::Longitude, QString("%1").arg(longitude));

    return vCardProperty(VC_GEOGRAPHIC_POSITION, values, params);
}

vCardProperty vCardProperty::createName(const QString& firstname, const QString& lastname, const QString& additional, const QString& prefix, const QString& suffix, const vCardParamList& params)
{
    QStringList values;
    values.insert(vCardProperty::Lastname, lastname);
    values.insert(vCardProperty::Firstname, firstname);
    values.insert(vCardProperty::Additional, additional);
    values.insert(vCardProperty::Prefix, prefix);
    values.insert(vCardProperty::Suffix, suffix);

    return vCardProperty(VC_NAME, values, params);
}

vCardProperty vCardProperty::createdFormattedName(const QString& name)
{
    QStringList values;
    values.append(name);

    return vCardProperty(VC_FORMATTED_NAME, values);
}

vCardProperty vCardProperty::createKHID(const QString& khID, const vCardParamList& params)
{
    QStringList values;
    values.append(khID);

    return vCardProperty(VC_KHID, values, params);
}

vCardProperty vCardProperty::createPublicKey(const QString& publicKey, const vCardParamList& params)
{
    QStringList values;
    values.append(publicKey);

    return vCardProperty(VC_KH_PUBLIC_KEY, values, params);
}

vCardProperty vCardProperty::createOrganization(const QString& name, const QStringList& levels, const vCardParamList& params)
{
    QStringList values;
    values.append(name);
    values.append(levels);

    return vCardProperty(VC_ORGANIZATION, values, params);
}

vCardProperty vCardProperty::createNotes(const QString& notes)
{
  /** \warning Multiline notes don't work properly on Windows 7
      "Windows system contact" application doesn't split multilines text.      
      It looks like it is a problem on the "Windows system contact" side
   */  
  QString notesTmp = notes;
  // "\n" is new line in the vCard standard.
  notesTmp.replace("\n", "\\n");
  QStringList values;
  values.append(notesTmp);

  return vCardProperty(VC_NOTE, values);
}

vCardProperty vCardProperty::createAvatar(const std::vector<char>& iconData)
{
  QByteArray byteArray(iconData.data(), iconData.size());
  QString iconBase64 = QString::fromLatin1(byteArray.toBase64().data());
  QString propertuName = VC_PHOTO + QString(";PNG")/*icon format*/ + ";ENCODING=BASE64";
  return vCardProperty(propertuName, iconBase64);
}