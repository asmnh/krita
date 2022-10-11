/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISOPACITYOPTION_H
#define KISOPACITYOPTION_H

#include <KisStandardOptions.h>

class KisPainter;

class KisOpacityOption : public KisStandardOption<KisOpacityOptionData>
{
public:
    using BaseClass = KisStandardOption<KisOpacityOptionData>;

    using BaseClass::BaseClass;
    using BaseClass::apply;

    quint8 apply(KisPainter* painter, const KisPaintInformation& info) const;
};

#endif // KISOPACITYOPTION_H
