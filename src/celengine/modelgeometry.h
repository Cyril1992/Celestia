// modelgeometry.h
//
// Copyright (C) 2010, Celestia Development Team
// Original version by Chris Laurel <claurel@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#pragma once

#include <memory>

#include <Eigen/Geometry>

#include <celmodel/model.h>
#include <celutil/resmanager.h>
#include "geometry.h"


class ModelOpenGLData;

class ModelGeometry : public Geometry
{
 public:
    ModelGeometry(std::unique_ptr<cmod::Model>&& model);
    ~ModelGeometry() = default;

    /*! Find the closest intersection between the ray and the
     *  model.  If the ray intersects the model, return true
     *  and set distance; otherwise return false and leave
     *  distance unmodified.
     */
    virtual bool pick(const Eigen::ParametrizedLine<double, 3>& r, double& distance) const;

    //! Render the model in the current OpenGL context
    virtual void render(RenderContext&, double t = 0.0);

    virtual bool usesTextureType(cmod::Material::TextureSemantic) const;
    virtual bool isOpaque() const;
    virtual bool isNormalized() const;

    void loadTextures();

 private:
    std::unique_ptr<cmod::Model> m_model;
    bool m_vbInitialized{ false };
    std::unique_ptr<ModelOpenGLData> m_glData;
};
