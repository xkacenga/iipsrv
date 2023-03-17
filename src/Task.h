/*
    IIP Generic Task Class

    Copyright (C) 2006-2021 Ruven Pillay

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _TASK_H
#define _TASK_H

#include <string>
#include <tuple>
#include <vector>

#include "IIPImage.h"
#include "IIPResponse.h"
#include "JPEGCompressor.h"
#include "View.h"
#include "TileManager.h"
#include "Timer.h"
#include "Writer.h"
#include "Cache.h"
#include "Watermark.h"
#include "Transforms.h"
#include "Logger.h"
#include "PNGCompressor.h"

// Define our http header cache max age (24 hours)
#define MAX_AGE 86400

#ifdef HAVE_EXT_POOL_ALLOCATOR
#include <ext/pool_allocator.h>
typedef HASHMAP<std::string, IIPImage,
                __gnu_cxx::hash<const std::string>,
                std::equal_to<const std::string>,
                __gnu_cxx::__pool_alloc<std::pair<const std::string, IIPImage>>>
    imageCacheMapType;
#else
typedef HASHMAP<std::string, IIPImage> imageCacheMapType;
#endif

/// Structure to hold our session data
struct Session
{
  IIPImage **image;
  JPEGCompressor *jpeg;
  PNGCompressor *png;
  View *view;
  IIPResponse *response;
  Watermark *watermark;
  Transform *processor;
  int loglevel;
  Logger *logfile;
  std::map<const std::string, std::string> headers;
  std::map<const std::string, unsigned int> codecOptions;

  imageCacheMapType *imageCache;
  Cache *tileCache;

#ifdef DEBUG
  FileWriter *out;
#else
  FCGIWriter *out;
#endif
};

/// Generic class to encapsulate various commands
class Task
{

protected:
  /// Timer for each task
  Timer command_timer;

  /// Pointer to our session data
  Session *session;

  /// Argument supplied to the task
  std::string argument;

public:
  /// Virtual destructor
  virtual ~Task() { ; };

  /// Main public function
  virtual void run(Session *session, const std::string &argument) { ; };

  /// Factory function
  /** @param type command type */
  static Task *factory(const std::string &type);

  /// Check image
  void checkImage();
};

/// OBJ commands
class OBJ : public Task
{

public:
  void run(Session *session, const std::string &argument);

  void iip();
  void iip_server();
  void max_size();
  void resolution_number();
  void colorspace(std::string arg);
  void tile_size();
  void bits_per_channel();
  void horizontal_views();
  void vertical_views();
  void min_max_values();
  void resolutions();
  void metadata(std::string field);
};

/// JPEG Quality Command
class QLT : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// SDS Command
class SDS : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// MINMAX Command
class MINMAX : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Contrast Command
class CNT : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Gamma Command
class GAM : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// CVT Width Command
class WID : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// CVT Height Command
class HEI : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// CVT Region Command
class RGN : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// ROT Rotation Command
class ROT : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// FIF Command
class FIF : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// JPEG Tile Export Command
class JTL : public Task
{
public:
  void run(Session *session, const std::string &argument);

  /// Send out a single tile
  /** @param session our current session
      @param resolution requested image resolution
      @param tile requested tile index
   */
  void send(Session *session, int resolution, int tile);
};

typedef struct CompressedTile
{
  RawTile rawtile;
  unsigned int compressedLen;
} CompressedTile;

// JPEG Tile Export Command - extended
class JTL_Ext : public Task
{
public:
  void run(Session *session, const std::string &argument);

  CompressedTile getTile(Session *session, int resolution, int tile);

  void send(Compressor *compressor,
            const std::vector<CompressedTile> &compressedTiles,
            const std::vector<int> &invalidPathIndices);
  void sendZip(Compressor *compressor,
            const std::vector<CompressedTile> &compressedTiles,
            const std::vector<int> &invalidPathIndices);
};

/// PNG Tile Command
class PTL : public JTL
{
public:
  void run(Session *session, const std::string &argument)
  {
    // Set our encoding format and call JTL::run
    session->view->output_format = PNG;
    JTL::run(session, argument);
  };
};

/// JPEG Tile Sequence Command
class JTLS : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Tile Command
class TIL : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// CVT Region Export Command
class CVT : public Task
{
public:
  void run(Session *session, const std::string &argument);

  /// Send out our requested region
  /** @param session our current session */
  void send(Session *session);
};

/// ICC Profile Command
class ICC : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Shading Command
class SHD : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Colormap Command
class CMP : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Inversion Command
class INV : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Zoomify Request Command
class Zoomify : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// SPECTRA Request Command
class SPECTRA : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// SPECTRA Request Command
class PFL : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Quality Layers Command
class LYR : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// DeepZoom Request Command
class DeepZoom : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// DeepZoomExt Request Command
class DeepZoomExt : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// IIIF Command
class IIIF : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Color Twist Command
class CTW : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

/// Color Conversion Command
class COL : public Task
{
public:
  void run(Session *session, const std::string &argument);
};

#endif
