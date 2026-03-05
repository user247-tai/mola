/*               _
 _ __ ___   ___ | | __ _
| '_ ` _ \ / _ \| |/ _` | Modular Optimization framework for
| | | | | | (_) | | (_| | Localization and mApping (MOLA)
|_| |_| |_|\___/|_|\__,_| https://github.com/MOLAorg/mola

 Copyright (C) 2018-2026 Jose Luis Blanco, University of Almeria,
                         and individual contributors.
 SPDX-License-Identifier: GPL-3.0
 See LICENSE for full license information.
*/
/**
 * @file   MolaVizImGui_handlers.cpp
 * @brief  Sensor-observation GUI handlers for the Dear ImGui backend.
 *
 * Each handler renders its observation type into a named ImGui sub-window.
 * The FBO-backed GL canvas approach (render mrpt scene → texture → ImGui
 * Image) is the intended final design; stubs show the structure.
 *
 * TODO items are marked with TODO comments.
 *
 * @author Jose Luis Blanco Claraco
 * @date   2026
 */

#include <mola_viz_imgui/MolaVizImGui.h>
#include <mrpt/obs/CObservation2DRangeScan.h>
#include <mrpt/obs/CObservation3DRangeScan.h>
#include <mrpt/obs/CObservationGPS.h>
#include <mrpt/obs/CObservationIMU.h>
#include <mrpt/obs/CObservationImage.h>
#include <mrpt/obs/CObservationPointCloud.h>
#include <mrpt/obs/CObservationRotatingScan.h>
#include <mrpt/obs/CObservationVelodyneScan.h>

using namespace mola;

// ---------------------------------------------------------------------------
// Helper: show common sensor metadata as ImGui::Text lines.
// `subWindowTitle` is used as the ImGui window key.
// ---------------------------------------------------------------------------

namespace
{

void show_common_sensor_info(const mrpt::obs::CObservation& obs, const std::string& subWindowTitle)
{
  // Rate estimation — one low-pass filter per (subWindowTitle, sensor class):
  using key_t = std::pair<std::string, std::string>;
  static std::map<key_t, double> lastTimestamp;
  static std::map<key_t, double> estimatedHz;

  const key_t      key    = {subWindowTitle, obs.GetRuntimeClass()->className};
  const double     curTim = mrpt::Clock::toDouble(obs.timestamp);
  constexpr double alpha  = 0.9;

  double showHz = 0.0;
  if (lastTimestamp.count(key))
  {
    const double At    = curTim - lastTimestamp[key];
    const double curHz = At > 0.0 ? 1.0 / At : 0.0;
    auto&        est   = estimatedHz[key];
    est                = alpha * est + (1.0 - alpha) * curHz;
    showHz             = est;
  }
  lastTimestamp[key] = curTim;

  ImGui::Text("Timestamp: %s", mrpt::system::dateTimeToString(obs.timestamp).c_str());
  if (showHz > 0.0)
    ImGui::Text("Rate: %.2f Hz  |  Class: %s", showHz, obs.GetRuntimeClass()->className);

  mrpt::poses::CPose3D sensorPose;
  obs.getSensorPose(sensorPose);
  ImGui::Text("Sensor pose: %s", sensorPose.asString().c_str());
}

// ---------------------------------------------------------------------------
// CObservationImage / CObservation3DRangeScan (intensity channel)
// ---------------------------------------------------------------------------

void handler_images(
    const mrpt::rtti::CObject::Ptr& o, void* /*handle*/,
    const MolaVizImGui::window_name_t& /*parentWin*/, MolaVizImGui* /*instance*/,
    const mrpt::containers::yaml* /*extra*/)
{
  // TODO: upload mrpt::img::CImage to an OpenGL texture and show via
  // ImGui::Image(reinterpret_cast<ImTextureID>(tex_id), size).
  //
  // Skeleton:
  //   auto obs = std::dynamic_pointer_cast<mrpt::obs::CObservationImage>(o);
  //   if (!obs) return;
  //   obs->load();
  //   static std::map<std::string, GLuint> tex_cache;
  //   GLuint& tex = tex_cache[subWindowTitle];
  //   upload_cimage_to_gl_texture(obs->image, tex);
  //   ImGui::Image(reinterpret_cast<ImTextureID>(tex), ImVec2(w, h));

  if (ImGui::Begin("##img_stub"))
  {
    ImGui::TextDisabled("(Image handler: TODO — upload CImage to GL texture)");
    if (auto obs = std::dynamic_pointer_cast<mrpt::obs::CObservation>(o); obs)
      show_common_sensor_info(*obs, "image");
  }
  ImGui::End();
}

// ---------------------------------------------------------------------------
// Point cloud observations
// ---------------------------------------------------------------------------

void handler_point_cloud(
    const mrpt::rtti::CObject::Ptr& o, void* /*handle*/,
    const MolaVizImGui::window_name_t& /*parentWin*/, MolaVizImGui* /*instance*/,
    const mrpt::containers::yaml* /*extra*/)
{
  // TODO: render the point cloud into a per-subwindow FBO (same pattern as
  // render_background_scene) and display as ImGui::Image.
  //
  // For now: show a text summary.

  auto obs = std::dynamic_pointer_cast<mrpt::obs::CObservation>(o);
  if (!obs) return;

  const std::string title = std::string(obs->GetRuntimeClass()->className) + "##pc";
  if (ImGui::Begin(title.c_str()))
  {
    show_common_sensor_info(*obs, title);

    if (auto objPc = std::dynamic_pointer_cast<mrpt::obs::CObservationPointCloud>(o); objPc)
    {
      if (objPc->pointcloud)
        ImGui::Text(
            "Points: %zu  |  Type: %s", objPc->pointcloud->size(),
            objPc->pointcloud->GetRuntimeClass()->className);
      else
        ImGui::TextDisabled("(no point cloud data)");
    }
    else
    {
      ImGui::TextDisabled("(point cloud handler: TODO — FBO render)");
    }
  }
  ImGui::End();
}

// ---------------------------------------------------------------------------
// CObservationGPS
// ---------------------------------------------------------------------------

void handler_gps(
    const mrpt::rtti::CObject::Ptr& o, void* /*handle*/,
    const MolaVizImGui::window_name_t& /*parentWin*/, MolaVizImGui* /*instance*/,
    const mrpt::containers::yaml* /*extra*/)
{
  auto obj = std::dynamic_pointer_cast<mrpt::obs::CObservationGPS>(o);
  if (!obj) return;

  if (ImGui::Begin("GPS##mola"))
  {
    show_common_sensor_info(*obj, "GPS");

    if (auto* gga = obj->getMsgByClassPtr<mrpt::obs::gnss::Message_NMEA_GGA>(); gga)
    {
      ImGui::Text("Latitude:  %.6f deg", gga->fields.latitude_degrees);
      ImGui::Text("Longitude: %.6f deg", gga->fields.longitude_degrees);
      ImGui::Text("Altitude:  %.2f m", gga->fields.altitude_meters);
      ImGui::Text("HDOP:      %.2f", gga->fields.HDOP);
      ImGui::Text(
          "UTC: %02u:%02u:%05.2f", static_cast<unsigned>(gga->fields.UTCTime.hour),
          static_cast<unsigned>(gga->fields.UTCTime.minute), gga->fields.UTCTime.sec);
    }
    if (obj->covariance_enu.has_value())
    {
      const auto& cov = obj->covariance_enu.value();
      ImGui::Text(
          "sigma [m]: x=%.2f  y=%.2f  z=%.2f", std::sqrt(cov(0, 0)), std::sqrt(cov(1, 1)),
          std::sqrt(cov(2, 2)));
    }
  }
  ImGui::End();
}

// ---------------------------------------------------------------------------
// CObservationIMU
// ---------------------------------------------------------------------------

void handler_imu(
    const mrpt::rtti::CObject::Ptr& o, void* /*handle*/,
    const MolaVizImGui::window_name_t& /*parentWin*/, MolaVizImGui* /*instance*/,
    const mrpt::containers::yaml* /*extra*/)
{
  auto obj = std::dynamic_pointer_cast<mrpt::obs::CObservationIMU>(o);
  if (!obj) return;

  if (ImGui::Begin("IMU##mola"))
  {
    show_common_sensor_info(*obj, "IMU");

    if (obj->has(mrpt::obs::IMU_WX))
      ImGui::Text(
          "omega: (%.4f, %.4f, %.4f)", obj->get(mrpt::obs::IMU_WX), obj->get(mrpt::obs::IMU_WY),
          obj->get(mrpt::obs::IMU_WZ));
    else
      ImGui::TextDisabled("omega: N/A");

    if (obj->has(mrpt::obs::IMU_X_ACC))
      ImGui::Text(
          "accel: (%.4f, %.4f, %.4f)", obj->get(mrpt::obs::IMU_X_ACC),
          obj->get(mrpt::obs::IMU_Y_ACC), obj->get(mrpt::obs::IMU_Z_ACC));
    else
      ImGui::TextDisabled("accel: N/A");
  }
  ImGui::End();
}

}  // namespace

// ---------------------------------------------------------------------------
// Registration — called from MRPT_INITIALIZER in MolaVizImGui.cpp
// ---------------------------------------------------------------------------

void mola_viz_imgui_register_default_handlers()
{
  // clang-format off
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservationImage",        &handler_images);
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservation3DRangeScan",  &handler_images);
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservationGPS",          &handler_gps);
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservationIMU",          &handler_imu);
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservationPointCloud",   &handler_point_cloud);
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservation3DRangeScan",  &handler_point_cloud);
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservation2DRangeScan",  &handler_point_cloud);
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservationRotatingScan", &handler_point_cloud);
  MolaVizImGui::register_gui_handler("mrpt::obs::CObservationVelodyneScan", &handler_point_cloud);
  // clang-format on
}
