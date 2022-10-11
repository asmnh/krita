/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFade2.h"

// TODO: remove!
#include <kis_dynamic_sensor.h>

#include <kis_paint_information.h>
#include <KisCurveOptionData.h>


KisDynamicSensorFade2::KisDynamicSensorFade2(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(FadeId, data, curveOverride)
    , m_periodic(data.isPeriodic)
    , m_length(data.length)
{
}

qreal KisDynamicSensorFade2::value(const KisPaintInformation &pi) const
{
    if (pi.isHoveringMode()) return 1.0;

    const int currentValue =
        m_periodic ?
        pi.currentDabSeqNo() % m_length :
        qMin(pi.currentDabSeqNo(), m_length);

    return qreal(currentValue) / m_length;
}
