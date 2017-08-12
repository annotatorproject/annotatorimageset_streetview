// Copyright 2017 Annotator Team

#ifndef ANNOTATOR_ANNOTATORLIB_STREETVIEW_H
#define ANNOTATOR_ANNOTATORLIB_STREETVIEW_H

/************************************************************
 ImageFolder class header
 ************************************************************/

#include "AnnotatorLib/ImageSet/AbstractImageSet.h"

#include <string>
#include <vector>

#include <AnnotatorLib/AnnotatorLibDatastructs.h>
#include <AnnotatorLib/annotatorlib_api.h>

#include <kml/dom.h>
#include <opencv2/opencv.hpp>

//#ifndef STREETVIEW_API_KEY
#define STREETVIEW_API_KEY ""
//#endif

namespace AnnotatorLib {
namespace ImageSet {
/************************************************************/
/**
 *
 */
class ANNOTATORLIB_API StreetView : public AbstractImageSet {
 public:
  StreetView(std::string path);

  /**
   *
   * @return type
   */
  virtual ImageSetType getType() override;

  /**
   *
   * @param frame
   * @return image
   */
  virtual Image getImage(unsigned long position) override;

  virtual std::string getImagePath(unsigned long frame) override;

  virtual bool gotoPosition(unsigned long position) override;

  virtual long getPosition() override;

  /**
   *
   * @return next
   */
  virtual bool hasNext() override;

  /**
   *
   * @return image
   */
  virtual Image next() override;

  /**
   *
   * @return size
   */
  virtual unsigned int size() override;

  virtual unsigned int getFPS() override;

  virtual std::string getPath() override;

  virtual bool equals(std::shared_ptr<AbstractImageSet> other) override;

 protected:
  void init();
  void parseFeature(const kmldom::FeaturePtr &feature);
  void parsePlacemarks(const kmldom::FeaturePtr &feature);
  void placemarkToUrl(const kmldom::PlacemarkPtr &placemark);
  void geometryToUrl(const kmldom::GeometryPtr &geometry);
  void pointToUrl(const kmldom::PointPtr &point);
  void flyToPointerToUrl(const kmldom::GxFlyToPtr &point);

  Image urlToImage(std::string url);

  long position = 0;
  std::string path;

  std::vector<std::string> urls;
};
/************************************************************/
/* External declarations (package visibility)               */
/************************************************************/

/* Inline functions                                         */
}  // of namespace ImageSet
}  // of namespace AnnotatorLib

/************************************************************
 End of Camera class header
 ************************************************************/

#endif
