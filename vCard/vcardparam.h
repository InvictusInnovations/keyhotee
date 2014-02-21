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

#ifndef VCARDPARAM_H
#define VCARDPARAM_H

#include "libvcard_global.h"
#include <QString>

class vCardParam
{
public:
    enum vCardParamGroup
    {
        Type,
        Encoding,
        Charset,
        Undefined
    };

protected:
    vCardParamGroup m_group;
    QString m_value;

public:
    vCardParam();
    vCardParam(const QString& value, vCardParamGroup group = vCardParam::Undefined);
    ~vCardParam();

    vCardParamGroup group() const;
    QString value() const;
    bool isValid() const;

    bool operator== (const vCardParam& param) const;
    bool operator!= (const vCardParam& param) const;

    QByteArray toByteArray(vCardVersion version = VC_VER_2_1) const;

    static QByteArray toByteArray(QList<vCardParam> params, vCardVersion version = VC_VER_2_1);
    static QList<vCardParam> fromByteArray(const QByteArray& data);
};

typedef QList<vCardParam> vCardParamList;

#endif // VCARDPARAM_H
