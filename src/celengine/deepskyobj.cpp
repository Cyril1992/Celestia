// deepskyobj.cpp
//
// Copyright (C) 2003-2009, the Celestia Development Team
// Original version by Chris Laurel <claurel@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include "deepskyobj.h"

#include <cmath>

#include <celastro/astro.h>
#include <celmath/intersect.h>
#include <celmath/sphere.h>
#include "hash.h"

namespace astro = celestia::astro;
namespace math = celestia::math;

Eigen::Vector3d DeepSkyObject::getPosition() const
{
    return position;
}

void DeepSkyObject::setPosition(const Eigen::Vector3d& p)
{
    position = p;
}

Eigen::Quaternionf DeepSkyObject::getOrientation() const
{
    return orientation;
}

void DeepSkyObject::setOrientation(const Eigen::Quaternionf& q)
{
    orientation = q;
}

void DeepSkyObject::setRadius(float r)
{
    radius = r;
}

float DeepSkyObject::getAbsoluteMagnitude() const
{
    return absMag;
}

void DeepSkyObject::setAbsoluteMagnitude(float _absMag)
{
    absMag = _absMag;
}

std::string DeepSkyObject::getDescription() const
{
    return "";
}

const std::string& DeepSkyObject::getInfoURL() const
{
    return infoURL;
}

void DeepSkyObject::setInfoURL(const std::string& s)
{
    infoURL = s;
}


bool DeepSkyObject::pick(const Eigen::ParametrizedLine<double, 3>& ray,
                         double& distanceToPicker,
                         double& cosAngleToBoundCenter) const
{
    return isVisible() && math::testIntersection(ray,
                                                 math::Sphered(position, static_cast<double>(radius)),
                                                 distanceToPicker,
                                                 cosAngleToBoundCenter);
}


bool DeepSkyObject::load(const AssociativeArray* params, const fs::path& resPath)
{
    // Get position
    if (auto pos = params->getLengthVector<double>("Position", astro::KM_PER_LY<double>); pos.has_value())
    {
        setPosition(*pos);
    }
    else
    {
        auto distance = params->getLength<double>("Distance", astro::KM_PER_LY<double>).value_or(1.0);
        auto RA = params->getAngle<double>("RA", astro::DEG_PER_HRA).value_or(0.0);
        auto dec = params->getAngle<double>("Dec").value_or(0.0);

        Eigen::Vector3d p = astro::equatorialToCelestialCart(RA, dec, distance);
        setPosition(p);
    }

    // Get orientation
    auto axis = params->getVector3<double>("Axis").value_or(Eigen::Vector3d::UnitX());
    auto angle = params->getAngle<double>("Angle").value_or(0.0);

    setOrientation(Eigen::Quaternionf(Eigen::AngleAxisf(static_cast<float>(math::degToRad(angle)),
                                                        axis.cast<float>().normalized())));

    setRadius(params->getLength<float>("Radius", astro::KM_PER_LY<double>).value_or(1.0f));

    if (auto absMagValue = params->getNumber<float>("AbsMag"); absMagValue.has_value())
        setAbsoluteMagnitude(*absMagValue);

    // FIXME: infourl class
    if (const std::string* infoURLValue = params->getString("InfoURL"); infoURLValue != nullptr)
    {
        std::string modifiedURL;
        if (infoURLValue->find(':') == std::string::npos)
        {
            // Relative URL, the base directory is the current one,
            // not the main installation directory
            if (resPath.c_str()[1] == ':')
                // Absolute Windows path, file:/// is required
                modifiedURL = "file:///" + resPath.string() + "/" + *infoURLValue;
            else if (!resPath.empty())
                modifiedURL = resPath.string() + "/" + *infoURLValue;
        }
        setInfoURL(modifiedURL.empty() ? *infoURLValue : modifiedURL);
    }

    if (auto visibleValue = params->getBoolean("Visible"); visibleValue.has_value())
    {
        setVisible(*visibleValue);
    }

    if (auto clickableValue = params->getBoolean("Clickable"); clickableValue.has_value())
    {
        setClickable(*clickableValue);
    }

    return true;
}
