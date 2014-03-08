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

#include "vcardparam.h"
#include <QStringList>

#define VC_GROUP_TOKEN "%1=%2"
#define VC_TYPE_TOKEN "TYPE"
#define VC_TYPE_SEP_TOKEN ','
#define VC_ENCODING_TOKEN "ENCODING"
#define VC_CHARSET_TOKEN "CHARSET"

vCardParam::vCardParam()
    :   m_group(vCardParam::Undefined)
{
}

vCardParam::vCardParam(const QString& value, vCardParamGroup group)
    :   m_group(group),
        m_value(value)
{
}

vCardParam::~vCardParam()
{
}

vCardParam::vCardParamGroup vCardParam::group() const
{
    return m_group;
}

QString vCardParam::value() const
{
    return m_value;
}

bool vCardParam::isValid() const
{
    return !m_value.isEmpty();
}

bool vCardParam::operator== (const vCardParam& param) const
{
    return ((m_group == param.group()) && (m_value == param.value()));
}

bool vCardParam::operator!= (const vCardParam& param) const
{
    return ((m_group != param.group()) || (m_value != param.value()));
}

QByteArray vCardParam::toByteArray(vCardVersion version) const
{
    QByteArray buffer;

    switch (version)
    {
        case VC_VER_2_1:
        {
            switch (m_group)
            {
                case vCardParam::Charset:
                    buffer.append(QString(VC_GROUP_TOKEN).arg(VC_CHARSET_TOKEN).arg(m_value));
                    break;

                case vCardParam::Encoding:
                    buffer.append(QString(VC_GROUP_TOKEN).arg(VC_ENCODING_TOKEN).arg(m_value));
                    break;

                default:
                    buffer.append(m_value);
                    break;
            }
            break;
        }
        break;

        case VC_VER_3_0:
        {
            switch (m_group)
            {
                case vCardParam::Type:
                    buffer.append(QString(VC_GROUP_TOKEN).arg(VC_TYPE_TOKEN).arg(m_value));
                    break;

                case vCardParam::Charset:
                    buffer.append(QString(VC_GROUP_TOKEN).arg(VC_CHARSET_TOKEN).arg(m_value));
                    break;

                case vCardParam::Encoding:
                    buffer.append(QString(VC_GROUP_TOKEN).arg(VC_ENCODING_TOKEN).arg(m_value));
                    break;

                default:
                    buffer.append(m_value);
                    break;
            }
        }
        break;

        default:
            break;
    }

    return buffer.toUpper();
}

QByteArray vCardParam::toByteArray(QList<vCardParam> params, vCardVersion version)
{
    QByteArray buffer;

    switch (version)
    {
        case VC_VER_2_1:
        {
            QStringList ps;
            for (const vCardParam& param : params)
                ps.append(param.toByteArray(VC_VER_2_1));
            buffer.append(ps.join(QString(VC_SEPARATOR_TOKEN)));
        }
        break;

        case VC_VER_3_0:
        {
            QStringList types;
            QStringList encodings;
            QStringList charsets;
            QStringList unknowns;
            for (const vCardParam& param : params)
            {
                QByteArray param_str = param.toByteArray(VC_VER_2_1);
                switch (param.group())
                {
                    case Type:
                        types.append(param_str);
                        break;

                    case Encoding:
                        encodings.append(param_str);
                        break;

                    case Charset:
                        charsets.append(param_str);
                        break;

                    default:
                        unknowns.append(param_str);
                }
            }

            unknowns += charsets;
            unknowns += encodings;

            if (!types.isEmpty())
                unknowns.prepend(QString(VC_GROUP_TOKEN).arg(VC_TYPE_TOKEN).arg(types.join(QString(VC_TYPE_SEP_TOKEN))));

            if (!unknowns.isEmpty())
                buffer.append(unknowns.join(QString(VC_SEPARATOR_TOKEN)));
        }
        break;

        default:
            break;
    }

    return buffer.toUpper();
}

QList<vCardParam> vCardParam::fromByteArray(const QByteArray& data)
{
    QList<vCardParam> params;

    QStringList tokens = QString::fromUtf8(data).simplified().split(VC_SEPARATOR_TOKEN);
    for (const QString token : tokens)
    {
	    int token_size = token.count();
        if (token.startsWith(VC_TYPE_TOKEN))
            for (const QString& type : token.right(token_size-5).split(VC_TYPE_SEP_TOKEN))
                params.append(vCardParam(type, vCardParam::Type));

        else if (token.startsWith(VC_ENCODING_TOKEN))
            params.append(vCardParam(token.right(token_size-9), vCardParam::Encoding));

        else if (token.startsWith(VC_CHARSET_TOKEN))
            params.append(vCardParam(token.right(token_size-8), vCardParam::Charset));

        else
            params.append(vCardParam(token));
    }

    return params;
}
