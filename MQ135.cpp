/**************************************************************************/
/*!
@file     MQ135.cpp
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3

First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#include "MQ135.h"

/**************************************************************************/
/*!
@brief  Default constructor

@param[in] pin  The analog input pin for the readout of the sensor
*/
/**************************************************************************/

MQ135::MQ135(uint8_t pin) {
  _pin = pin;
  _rzero = RZERO;
  _rload = RLOAD;
  _atmoco2 = ATMOCO2;
}

/**************************************************************************/
/*!
@brief  constructor

@param[in] pin  The analog input pin for the readout of the sensor
@param[in] rl	The load resistance of you design in kOhm
@param[in] ppm  Atmospheric CO2 concentration (http://www.co2.earth)
*/
/**************************************************************************/
MQ135::MQ135(uint8_t pin, float rl, float ppm) {
  _pin = pin;
  _rzero = RZERO;
  _rload = rl;
  _atmoco2 = ppm;
}

/**************************************************************************/
/*!
@brief  Get the atmospehric CO2 concentrarion currently used

@return Atmospheric CO2 concentration used

*/
/**************************************************************************/
float MQ135::getAtmoco2()
{
	if (_atmoco2 == 0)
		return ATMOCO2;
	return _atmoco2;
}

/**************************************************************************/
/*!
@brief  Set the atmospehric CO2 concentrarion used in calibration

@param[in] ppm  Atmospheric CO2 concentration (ex https://www.co2.earth/)

*/
/**************************************************************************/
void MQ135::setAtmoco2(float ppm)
{
	if (ppm == 0)
		_atmoco2 = ATMOCO2;
	else
		_atmoco2 = ppm;
}

/**************************************************************************/
/*!
@brief  Get the load resistance currently used

@return load resistance currently used

*/
/**************************************************************************/
float MQ135::getRload()
{
	if (_rload == 0)
		return RLOAD;
	return _rload;
}

/**************************************************************************/
/*!
@brief  Set the load resistance value

@param[in] r  resistance value in kOhm (ex: 10.)

*/
/**************************************************************************/
void MQ135::setRload(float r)
{
	if (r == 0)
		_rload = RLOAD;
	else
		_rload = r;
}

/**************************************************************************/
/*!
@brief  Get rzero value

@return load resistance currently used

*/
/**************************************************************************/
float MQ135::getRzero()
{
	if (_rzero == 0.)
		return RZERO;
	return _rzero;
}

/**************************************************************************/
/*!
@brief  Set rzero value (obtained after calibration)

@param[in] r  resistance value in kOhm

*/
/**************************************************************************/
void MQ135::setRzero(float r)
{
	if (r == 0)
		_rzero = RZERO;
	else
		_rzero = r;
}

/**************************************************************************/
/*!
@brief  Get the correction factor to correct for temperature and humidity

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The calculated correction factor
*/
/**************************************************************************/
float MQ135::getCorrectionFactor(float t, float h) {
    // Linearization of the temperature dependency curve under and above 20 degree C
    // below 20degC: fact = a * t * t - b * t - (h - 33) * d
    // above 20degC: fact = a * t + b * h + c
    // this assumes a linear dependency on humidity
    if(t < 20){
        return CORA * t * t - CORB * t + CORC - (h-33.)*CORD;
    } else {
        return CORE * t + CORF * h + CORG;
    }
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value

@return The sensor resistance in kOhm
*/
/**************************************************************************/
float MQ135::getResistance() {
  int val = analogRead(_pin);
  return ((1023./(float)val) - 1.)*_rload;
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value corrected
        for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance kOhm
*/
/**************************************************************************/
float MQ135::getCorrectedResistance(float t, float h) {
  return getResistance()/getCorrectionFactor(t, h);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air)

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float MQ135::getPPM() {
  return PARA * pow((getResistance()/_rzero), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
        for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float MQ135::getCorrectedPPM(float t, float h) {
  return PARA * pow((getCorrectedResistance(t, h)/_rzero), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes

@return The sensor resistance RZero in kOhm
*/
/**************************************************************************/
float MQ135::measureRZero() {
  return getResistance() * pow((_atmoco2/PARA), (1./PARB));
}

/**************************************************************************/
/*!
@brief  Get the corrected resistance RZero of the sensor for calibration
        purposes

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance RZero in kOhm
*/
/**************************************************************************/
float MQ135::measureCorrectedRZero(float t, float h) {
  return getCorrectedResistance(t, h) * pow((_atmoco2/PARA), (1./PARB));
}
