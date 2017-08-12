// Copyright 2017 Annotator Team

#define Annotator_AnnotatorLib_StreetView_BODY

/************************************************************
 ImageFolder class body
 ************************************************************/

// include associated header file

#include "StreetView.h"

#include <functional>
#include <set>
#include <sstream>

#include <Poco/JSON/Handler.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <kml/base/file.h>
#include <kml/convenience/convenience.h>
#include <kml/engine.h>
#include <opencv2/opencv.hpp>

// Derived includes directives

namespace AnnotatorLib {
namespace ImageSet {

StreetView::StreetView(std::string path) {
  this->path = path;
  init();
}

ImageSetType StreetView::getType() { return ImageSetType::UNKNOWN; }

Image StreetView::getImage(unsigned long position) {
  Image img;
  try {
    if (position < urls.size()) img = urlToImage(urls[position]);
  } catch (std::exception &e) {
  }
  return img;
}

std::string StreetView::getImagePath(unsigned long frame) {
  return urls[frame];
}

bool StreetView::gotoPosition(unsigned long position) {
  if (urls.size() <= position) return false;
  this->position = position;
  return true;
}

long StreetView::getPosition() { return position; }

bool StreetView::hasNext() { return position + 1 < urls.size(); }

Image StreetView::next() {
  Image img;
  if (hasNext()) {
    img = getImage(position);
    position++;
  }
  return img;
}

unsigned int StreetView::size() { return this->urls.size(); }

unsigned int StreetView::getFPS() { return 10; }

std::string StreetView::getPath() { return path; }

bool StreetView::equals(std::shared_ptr<AbstractImageSet> other) {
  if (this == other.get()) return true;
  if (other->getType() != ImageSetType::CAMERA) return false;
  if (this->getPath() != other->getPath()) return false;
  return true;
}

void StreetView::parsePlacemarks(const kmldom::FeaturePtr &feature) {
  if (kmldom::PlacemarkPtr placemark = kmldom::AsPlacemark(feature)) {
    placemarkToUrl(placemark);
  } else if (const kmldom::ContainerPtr container =
                 kmldom::AsContainer(feature)) {
    for (size_t i = 0; i < container->get_feature_array_size(); ++i) {
      parsePlacemarks(container->get_feature_array_at(i));
    }
  }
}

void StreetView::geometryToUrl(const kmldom::GeometryPtr &geometry) {
  switch (geometry->Type()) {
    case kmldom::Type_Point:
      pointToUrl(kmldom::AsPoint(geometry));
      break;
    case kmldom::Type_LineString:
      break;
    case kmldom::Type_LinearRing:
      break;
    case kmldom::Type_Polygon:
      break;
    case kmldom::Type_MultiGeometry:
      break;
    case kmldom::Type_Model:
      break;
    default:
      break;
  }
  if (const kmldom::MultiGeometryPtr multigeometry =
          kmldom::AsMultiGeometry(geometry)) {
    for (size_t i = 0; i < multigeometry->get_geometry_array_size(); ++i) {
      geometryToUrl(multigeometry->get_geometry_array_at(i));
    }
  }
}

void StreetView::pointToUrl(const kmldom::PointPtr &point) {
  std::string lat = std::to_string(
      point->get_coordinates()->get_coordinates_array_at(0).get_latitude());
  std::string lon = std::to_string(
      point->get_coordinates()->get_coordinates_array_at(0).get_longitude());

  std::string heading = "0.0";
  std::string pitch = "0.0";
  try {
    heading = std::to_string(
        point->get_coordinates()->get_coordinates_array_at(0).get_heading());
    pitch = std::to_string(
        point->get_coordinates()->get_coordinates_array_at(0).get_pitch());
  } catch (...) {
  }

  std::string api_key = STREETVIEW_API_KEY;
  std::string url =
      "https://maps.googleapis.com/maps/api/"
      "streetview?size=600x300&key=" +
      api_key + "&heading=" + heading + "&pitch=" + pitch + "&location=" + lat +
      "," + lon;
  urls.push_back(url);
}

void StreetView::flyToPointerToUrl(const kmldom::GxFlyToPtr &point) {
  try {
    const kmldom::LookAtPtr &lookAt =
        kmldom::AsLookAt(point->get_abstractview());
    if (lookAt) {
      std::string lat = std::to_string(lookAt->get_latitude());
      std::string lon = std::to_string(lookAt->get_longitude());

      std::string heading = "0.0";
      if (lookAt->has_heading())
        heading = std::to_string(lookAt->get_heading());
      std::string pitch = "0.0";

      std::string api_key = STREETVIEW_API_KEY;
      std::string url =
          "https://maps.googleapis.com/maps/api/"
          "streetview?size=600x300&key=" +
          api_key + "&heading=" + heading + "&pitch=" + pitch + "&location=" +
          lat + "," + lon;
      urls.push_back(url);
    }
  } catch (...) {
  }
}

void StreetView::init() {
  // Read the file.
  std::string file_data;
  if (!kmlbase::File::ReadFileToString(path, &file_data)) {
    std::cout << path << " read failed" << std::endl;
    return;
  }

  // If the file was KMZ, extract the KML file.
  std::string kml;
  if (kmlengine::KmzFile::IsKmz(file_data)) {
    kmlengine::KmzFilePtr kmz_file = kmlengine::KmzFile::OpenFromString(path);
    if (!kmz_file) {
      std::cout << "Failed opening KMZ file" << std::endl;
      return;
    }
    if (!kmz_file->ReadKml(&kml)) {
      std::cout << "Failed to read KML from KMZ" << std::endl;
      return;
    }
  } else {
    kml = file_data;
  }

  // Parse it.
  std::string errors;
  kmlengine::KmlFilePtr kml_file =
      kmlengine::KmlFile::CreateFromParse(kml, &errors);
  if (!kml_file) {
    std::cout << errors << std::endl;
    return;
  }

  kmldom::FeaturePtr feature = kmlengine::GetRootFeature(kml_file->get_root());
  parseFeature(feature);
  parsePlacemarks(feature);
}

void StreetView::parseFeature(const kmldom::FeaturePtr &feature) {
  if (kmldom::GxTourPtr tour = kmldom::AsGxTour(feature)) {
    if (const kmldom::GxPlaylistPtr &playlist =
            kmldom::AsGxPlaylist(tour->get_gx_playlist())) {
      for (size_t i = 0; i < playlist->get_gx_tourprimitive_array_size(); ++i) {
        kmldom::GxFlyToPtr flyto =
            kmldom::AsGxFlyTo(playlist->get_gx_tourprimitive_array_at(i));
        if (flyto) {
          flyToPointerToUrl(flyto);
        }
      }
    }
  }
}

void StreetView::placemarkToUrl(const kmldom::PlacemarkPtr &placemark) {
  if (placemark->has_geometry()) geometryToUrl(placemark->get_geometry());
}

Image StreetView::urlToImage(std::string url) {
  Image img;
  try {
    Poco::URI uri(url);
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
                               uri.getPathAndQuery());
    session.sendRequest(req);
    Poco::Net::HTTPResponse res;
    std::istream &iStr = session.receiveResponse(res);
    std::string imageStr;
    Poco::StreamCopier::copyToString(iStr, imageStr);
    std::vector<uchar> vec(imageStr.begin(), imageStr.end());

    img = cv::imdecode(vec, cv::IMREAD_ANYCOLOR);
  } catch (...) {
  }

  return img;
}

// static attributes (if any)
}  // of namespace ImageSet
}  // of namespace AnnotatorLib

/************************************************************
 End of ImageFolder class body
 ************************************************************/
