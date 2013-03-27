#include "testphysicalquantitiesbase.h"

namespace BlackMiscTest {

/*!
 * \brief Constructor
 * \param parent
 */
TestPhysicalQuantitiesBase::TestPhysicalQuantitiesBase(QObject *parent) : QObject(parent)
{
    // void
}

/*!
 * Basic unit tests for physical units
 */
void TestPhysicalQuantitiesBase::unitsBasics()
{
    QVERIFY(CMeasurementPrefix::k() > CMeasurementPrefix::h());

    // some tests on units
    CDistanceUnit du1 = CDistanceUnit::m(); // Copy
    CDistanceUnit du2 = CDistanceUnit::m(); // Copy
    QVERIFY2(du1 == du2, "Compare by value 1");
    du1 = CDistanceUnit::m(); // Copy
    du2 = CDistanceUnit::m(); // Copy
    QVERIFY2(du1 == du2, "Compare by value 2");
    QVERIFY2(CDistanceUnit::m() == CDistanceUnit::m(), "Compare by value");
    QVERIFY2(CMeasurementPrefix::h() < CMeasurementPrefix::M(), "hecto < mega");

    CFrequencyUnit fu1 = CFrequencyUnit::Hz();
    QVERIFY2(fu1 != du1, "Hz must not be meter");
}

/*!
 * Distance tests
 */
void TestPhysicalQuantitiesBase::distanceBasics()
{
    CDistance d1(1,CDistanceUnit::m()); // 1m
    CDistance d2(100, CDistanceUnit::cm());
    CDistance d3(1.852 * 1000,CDistanceUnit::m()); // 1852m
    CDistance d4(1,CDistanceUnit::NM());

    QVERIFY2(d1 == d2, "1meter shall be 100cm");
    QVERIFY2(d3 == d4, "1852meters shall be 1NM");

    d3 *=2; // SI value
    d4 *=2.0; // SI value !
    QVERIFY2(d3 == d4, "2*1852meters shall be 2NM");

    // less / greater
    QVERIFY2(!(d1 < d2), "Nothing shall be less / greater");
    QVERIFY2(!(d1 > d2), "Nothing shall be less / greater");

    // epsilon tests
    d1.addUnitValue(d1.getUnit().getEpsilon()); // this should be still the same
    QVERIFY2(d1 == d2, "Epsilon: 1meter+epsilon shall be 100cm");
    QVERIFY2(!(d1 != d2), "Epsilon: 1meter+epsilon shall be 100cm");

    d1.addUnitValue(d1.getUnit().getEpsilon()); // now over epsilon threshold
    QVERIFY2(d1 != d2, "Epsilon exceeded: 1meter+2epsilon shall not be 100cm");
    QVERIFY2(d1 > d2, "d1 shall be greater");
}

/**
 * Unit tests for speed
 */
void TestPhysicalQuantitiesBase::speedBasics()
{
    CSpeed s1(100, CSpeedUnit::km_h());
    CSpeed s2(1000, CSpeedUnit::ft_min());
    QVERIFY2(s1.valueRounded(CSpeedUnit::NM_h(),0) == 54, qPrintable(QString("100km/h is not %1 NM/h").arg(s1.valueRounded(CSpeedUnit::NM_h(),0))));
    QVERIFY2(s2.valueRounded(CSpeedUnit::m_s(),1) == 5.1, qPrintable(QString("1000ft/min is not %1 m/s").arg(s2.valueRounded(CSpeedUnit::m_s(),1))));
}

/**
 * Frequency unit tests
 */
void TestPhysicalQuantitiesBase::frequencyTests()
{
    CFrequency f1(1, CFrequencyUnit::MHz());
    QVERIFY2(f1.valueRounded(CFrequencyUnit::kHz(),2) == 1000, "Mega is 1000kHz");
    QVERIFY2(f1 == 1000000 , "MHz is 1E6 Hz");
    CFrequency f2(CMeasurementPrefix::M(),CFrequencyUnit::Hz()) ; // 1 Megahertz
    QVERIFY2(f1 == f2 , "MHz is 1E6 Hz");
}

/**
 * Angle tests
 */
void TestPhysicalQuantitiesBase::angleTests()
{
    CAngle a1(180, CAngleUnit::deg());
    CAngle a2(1.5 * CAngle::pi(), CAngleUnit::rad());
    a2.switchUnit(CAngleUnit::deg());
    QVERIFY2(a2.unitValueToInteger() == 270, qPrintable(QString("1.5Pi should be 270deg, not %1 deg").arg(a2.unitValueToInteger())));
    QVERIFY2(a1.piFactor() == 1, qPrintable(QString("Pi should be 1PI,not %1").arg(a1.piFactor())));
}

/**
 * Weight tests
 */
void TestPhysicalQuantitiesBase::massTests()
{
    CMass w1(1000, CMassUnit::kg());
    CMass w2(w1, CMassUnit::kg());
    w2.switchUnit(CMassUnit::t());
    QVERIFY2(w2.unitValueToInteger() == 1, "1tonne shall be 1000kg");
    w2.switchUnit(CMassUnit::lb());
    QVERIFY2(w2.unitValueToDoubleRounded(2) == 2204.62, "1tonne shall be 2204pounds");
    QVERIFY2(w1 == w2, "Masses shall be equal");
}

/**
 * Pressure tests
 */
void TestPhysicalQuantitiesBase::pressureTests()
{
    CPressure p1(1013.25, CPressureUnit::hPa());
    CPressure p2(29.92,CPressureUnit::inHg());
    CPressure p3(29.92,CPressureUnit::inHgFL());
    CPressure p4(p1);
    p4.switchUnit(CPressureUnit::mbar());

    // does not match exactly
    QVERIFY2(p1 != p2, "Standard pressure test little difference");
    QVERIFY2(p1 == p3, "Standard pressure test matching");
    QVERIFY2(p1.unitValueToDouble() == p4.unitValueToDouble(), "mbar/hPa test");
}

/**
 * Temperature tests
 */
void TestPhysicalQuantitiesBase::temperatureTests()
{
    CTemperature t1(0, CTemperatureUnit::C()); // 0C
    CTemperature t2(1, CTemperatureUnit::F()); // 1F
    CTemperature t3(220.15, CTemperatureUnit::F());
    QVERIFY2(t1.convertedSiValueToDoubleRounded() == 273.15, qPrintable(QString("0C shall be 273.15K, not %1 K").arg(t1.convertedSiValueToDoubleRounded())));
    QVERIFY2(t2.valueRounded(CTemperatureUnit::C()) == -17.22, qPrintable(QString("1F shall be -17.22C, not %1 C").arg(t2.valueRounded(CTemperatureUnit::C()))));
    QVERIFY2(t3.valueRounded(CTemperatureUnit::C()) == 104.53, qPrintable(QString("220.15F shall be 104.53C, not %1 C").arg(t3.valueRounded(CTemperatureUnit::C()))));
}

/**
 * @brief Just testing obvious memory create / destruct flaws
 */
void TestPhysicalQuantitiesBase::memoryTests()
{
    CDistance* c = new CDistance(100, CDistanceUnit::m());
    c->switchUnit(CDistanceUnit::NM());
    QVERIFY2(c->getUnit() == CDistanceUnit::NM() && c->getConversionSiUnit() == CDistanceUnit::m(),
             "Testing distance units failed");
    delete c;

    CAngle* a = new CAngle(100, CAngleUnit::rad());
    a->switchUnit(CAngleUnit::deg());
    QVERIFY2(a->getUnit() == CAngleUnit::deg() && c->getConversionSiUnit() == CAngleUnit::rad(),
             "Testing angle units failed");
    delete a;
}

/**
 * @brief Just testing obvious memory create / destruct flaws
 */
void TestPhysicalQuantitiesBase::basicArithmetic()
{
    CPressure p1 = CPhysicalQuantitiesConstants::InternationalStandardSeaLevelPressure();
    CPressure p2(p1);
    p2 *= 2.0;
    CPressure p3 = p1 + p1;
    QVERIFY2(p3 == p2, "Pressure needs to be the same (2times)");
    p3 /= 2.0;
    QVERIFY2(p3 == p1, "Pressure needs to be the same (1time)");
    p3 = p3 - p3;
    QVERIFY2(p3.unitValueToDouble() == 0, "Value needs to be zero");
}


} // namespace
