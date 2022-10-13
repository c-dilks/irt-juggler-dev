#include "irtgeo/IrtGeoDRICH.h"

void IrtGeoDRICH::DD4hep_to_IRT() {

  // begin envelope
  /* FIXME: have no connection to GEANT G4LogicalVolume pointers; however all is needed
   * is to make them unique so that std::map work internally; resort to using integers,
   * who cares; material pointer can seemingly be '0', and effective refractive index
   * for all radiators will be assigned at the end by hand; FIXME: should assign it on
   * per-photon basis, at birth, like standalone GEANT code does;
   */
  auto nSectors       = det->constant<int>("DRICH_RECON_nSectors");
  auto vesselZmin     = det->constant<double>("DRICH_RECON_zmin");
  auto gasvolMaterial = det->constant<std::string>("DRICH_RECON_gasvolMaterial");
  TVector3 normX(1, 0,  0); // normal vectors
  TVector3 normY(0, -1, 0);
  auto surfEntrance = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, vesselZmin), normX, normY);
  for (int isec=0; isec<nSectors; isec++) {
    auto cv = irtGeometry->SetContainerVolume(
        irtDetector,             // Cherenkov detector
        "GasVolume",             // name
        isec,                    // path
        (G4LogicalVolume*)(0x0), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial (inaccessible?)
        surfEntrance             // surface
        );
    cv->SetAlternativeMaterialName(gasvolMaterial.c_str());
  }

  // photon detector
  // - FIXME: args (G4Solid,G4Material) inaccessible?
  auto cellMask = uint64_t(std::stoull(det->constant<std::string>("DRICH_RECON_cellMask")));
  CherenkovPhotonDetector* irtPhotonDetector = new CherenkovPhotonDetector(nullptr, nullptr);
  irtDetector->SetReadoutCellMask(cellMask);
  irtGeometry->AddPhotonDetector(
      irtDetector,      // Cherenkov detector
      nullptr,          // G4LogicalVolume (inaccessible?)
      irtPhotonDetector // photon detector
      );
  dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "cellMask = 0x%X", cellMask);

  // aerogel + filter
  /* AddFlatRadiator will create a pair of flat refractive surfaces internally;
   * FIXME: should make a small gas gap at the upstream end of the gas volume;
   * FIXME: do we need a sector loop?
   */
  auto aerogelZpos        = det->constant<double>("DRICH_RECON_aerogelZpos");
  auto aerogelThickness   = det->constant<double>("DRICH_RECON_aerogelThickness");
  auto aerogelMaterial    = det->constant<std::string>("DRICH_RECON_aerogelMaterial");
  auto filterZpos         = det->constant<double>("DRICH_RECON_filterZpos");
  auto filterThickness    = det->constant<double>("DRICH_RECON_filterThickness");
  auto filterMaterial     = det->constant<std::string>("DRICH_RECON_filterMaterial");
  auto aerogelFlatSurface = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, aerogelZpos), normX, normY);
  auto filterFlatSurface  = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, filterZpos),  normX, normY);
  for (int isec = 0; isec < nSectors; isec++) {
    auto aerogelFlatRadiator = irtGeometry->AddFlatRadiator(
        irtDetector,             // Cherenkov detector
        "Aerogel",               // name
        isec,                    // path
        (G4LogicalVolume*)(0x1), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial
        aerogelFlatSurface,      // surface
        aerogelThickness / dd4hep::mm // surface thickness
        );
    auto filterFlatRadiator = irtGeometry->AddFlatRadiator(
        irtDetector,             // Cherenkov detector
        "Filter",                // name
        isec,                    // path
        (G4LogicalVolume*)(0x2), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial
        filterFlatSurface,       // surface
        filterThickness / dd4hep::mm // surface thickness
        );
    aerogelFlatRadiator->SetAlternativeMaterialName(aerogelMaterial.c_str());
    filterFlatRadiator->SetAlternativeMaterialName(filterMaterial.c_str());
  }
  dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "aerogelZpos = %f cm", aerogelZpos);
  dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "filterZpos  = %f cm", filterZpos);
  dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "aerogel thickness = %f cm", aerogelThickness);
  dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "filter thickness  = %f cm", filterThickness);

  // sector loop
  for (int isec = 0; isec < nSectors; isec++) {
    std::string secName = "sec" + std::to_string(isec);

    // mirrors
    auto mirrorRadius = det->constant<double>("DRICH_RECON_mirrorRadius");
    dd4hep::Position mirrorCenter(
      det->constant<double>("DRICH_RECON_mirrorCenterX_"+secName),
      det->constant<double>("DRICH_RECON_mirrorCenterY_"+secName),
      det->constant<double>("DRICH_RECON_mirrorCenterZ_"+secName)
      );
    auto mirrorSphericalSurface  = new SphericalSurface(
        (1 / dd4hep::mm) * TVector3(mirrorCenter.x(), mirrorCenter.y(), mirrorCenter.z()), mirrorRadius / dd4hep::mm);
    auto mirrorOpticalBoundary = new OpticalBoundary(
        irtDetector->GetContainerVolume(), // CherenkovRadiator radiator
        mirrorSphericalSurface,            // surface
        false                              // bool refractive
        );
    irtDetector->AddOpticalBoundary(isec, mirrorOpticalBoundary);
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "");
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "  SECTOR %d MIRROR:", isec);
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "    mirror x = %f cm", mirrorCenter.x());
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "    mirror y = %f cm", mirrorCenter.y());
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "    mirror z = %f cm", mirrorCenter.z());
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "    mirror R = %f cm", mirrorRadius);

    // complete the radiator volume description; this is the rear side of the container gas volume
    irtDetector->GetRadiator("GasVolume")->m_Borders[isec].second = mirrorSphericalSurface;

    // sensor sphere (only used for validation of sensor normals)
    auto sensorSphRadius  = det->constant<double>("DRICH_RECON_sensorSphRadius");
    auto sensorThickness  = det->constant<double>("DRICH_RECON_sensorThickness");
    dd4hep::Position sensorSphCenter(
      det->constant<double>("DRICH_RECON_sensorSphCenterX_"+secName),
      det->constant<double>("DRICH_RECON_sensorSphCenterY_"+secName),
      det->constant<double>("DRICH_RECON_sensorSphCenterZ_"+secName)
      );
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "  SECTOR %d SENSOR SPHERE:", isec);
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "    sphere x = %f cm", sensorSphCenter.x());
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "    sphere y = %f cm", sensorSphCenter.y());
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "    sphere z = %f cm", sensorSphCenter.z());
    dd4hep::printout(dd4hep::ALWAYS, "IRTGEO", "    sphere R = %f cm", sensorSphRadius);

    // sensor modules: search the detector tree for sensors for this sector
    for(auto const& [de_name, detSensor] : detRich.children()) {
      if(de_name.find("sensor_de_"+secName)!=std::string::npos) {

        // get sensor info
        auto imodsec = detSensor.id();
        // - get sensor centroid position
        auto pvSensor  = detSensor.placement();
        auto posSensor = posRich + pvSensor.position();
        // - get sensor surface position
        dd4hep::Direction radialDir   = posSensor - sensorSphCenter; // sensor sphere radius direction
        auto posSensorSurface = posSensor + (radialDir.Unit() * (0.5*sensorThickness));
        // - get surface normal and in-plane vectors
        double sensorLocalNormX[3] = {1.0, 0.0, 0.0};
        double sensorLocalNormY[3] = {0.0, 1.0, 0.0};
        double sensorGlobalNormX[3], sensorGlobalNormY[3];
        pvSensor.ptr()->LocalToMasterVect(sensorLocalNormX, sensorGlobalNormX); // ignore vessel transformation, since it is a pure translation
        pvSensor.ptr()->LocalToMasterVect(sensorLocalNormY, sensorGlobalNormY);

        // validate sensor position and normal
        // - test normal vectors
        dd4hep::Direction normXdir, normYdir;
        normXdir.SetCoordinates(sensorGlobalNormX);
        normYdir.SetCoordinates(sensorGlobalNormY);
        auto normZdir   = normXdir.Cross(normYdir);         // sensor surface normal
        auto testOrtho  = normXdir.Dot(normYdir);           // should be zero, if normX and normY are orthogonal
        auto testRadial = radialDir.Cross(normZdir).Mag2(); // should be zero, if sensor surface normal is parallel to sensor sphere radius
        if(abs(testOrtho)>1e-6 || abs(testRadial)>1e-6) {
          dd4hep::printout(dd4hep::FATAL, "IRTGEO",
              "sensor normal is wrong: normX.normY = %f   |radialDir x normZdir|^2 = %f",
              testOrtho,
              testRadial
              );
          return;
        }
        // - test sensor positioning
        auto distSensor2center = sqrt((posSensorSurface-sensorSphCenter).Mag2()); // distance between sensor sphere center and sensor surface position
        auto testDist          = abs(distSensor2center-sensorSphRadius);          // should be zero, if sensor position w.r.t. sensor sphere center is correct
        if(abs(testDist)>1e-6) {
          dd4hep::printout(dd4hep::FATAL, "IRTGEO",
              "sensor positioning is wrong: dist(sensor, sphere_center) = %f,  sphere_radius = %f,  sensor_thickness = %f,  |diff| = %g\n",
              distSensor2center,
              sensorSphRadius,
              sensorThickness,
              testDist
              );
          return;
        }


        // create the optical surface
        auto sensorFlatSurface = new FlatSurface(
            (1 / dd4hep::mm) * TVector3(posSensorSurface.x(), posSensorSurface.y(), posSensorSurface.z()),
            TVector3(sensorGlobalNormX),
            TVector3(sensorGlobalNormY)
            );
        irtDetector->CreatePhotonDetectorInstance(
            isec,              // sector
            irtPhotonDetector, // CherenkovPhotonDetector
            imodsec,           // copy number
            sensorFlatSurface  // surface
            );
        // dd4hep::printout(dd4hep::ALWAYS, "IRTGEO",
        //     "sensor: id=0x%08X pos=(%5.2f, %5.2f, %5.2f) normX=(%5.2f, %5.2f, %5.2f) normY=(%5.2f, %5.2f, %5.2f)",
        //     imodsec,
        //     posSensorSurface.x(), posSensorSurface.y(), posSensorSurface.z(),
        //     normXdir.x(),  normXdir.y(),  normXdir.z(),
        //     normYdir.x(),  normYdir.y(),  normYdir.z()
        //     );
      }
    } // search for sensors

  } // sector loop

  // set refractive indices
  // FIXME: are these (weighted) averages? can we automate this?
  std::map<std::string, double> rIndices;
  rIndices.insert({"GasVolume", 1.00076});
  rIndices.insert({"Aerogel", 1.0190});
  rIndices.insert({"Filter", 1.5017});
  for (auto const& [rName, rIndex] : rIndices) {
    auto rad = irtDetector->GetRadiator(rName.c_str());
    if (rad)
      rad->SetReferenceRefractiveIndex(rIndex);
  }
}
