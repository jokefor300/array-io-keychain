#pragma once
#include <fc_light/shared_ptr.hpp>
#include <fc_light/filesystem.hpp>
#include <iostream>

namespace fc_light {
  class path;
class ofstream : virtual public std::ostream {
    public:
      enum mode { out, binary };
      ofstream();
      ofstream( const fc_light::path& file, int m = binary );
      ~ofstream();

      void open( const fc_light::path& file, int m = binary );
      size_t writesome( const char* buf, size_t len );
      size_t writesome(const std::shared_ptr<const char>& buffer, size_t len, size_t offset);
      void   put( char c );
      void   close();
      void   flush();

    private:
      class impl;
      fc_light::shared_ptr<impl> my;
  };

class ifstream : virtual public std::istream {
    public:
      enum mode { in, binary };
      enum seekdir { beg, cur, end };

      ifstream();
      ifstream( const fc_light::path& file, int m = binary);
      ~ifstream();

      void      open( const fc_light::path& file, int m );
      size_t    readsome( char* buf, size_t len );
      size_t    readsome(const std::shared_ptr<char>& buffer, size_t max, size_t offset);
      ifstream& read( char* buf, size_t len );
      ifstream& seekg( size_t p, seekdir d = beg );
      using std::istream::get;
      void      get( char& c ) { read( &c, 1 ); }
      void      close();
      bool      eof()const;
    private:
      class impl;
      fc_light::shared_ptr<impl> my;
  };

  /**
   * Grab the full contents of a file into a string object.
   * NB reading a full file into memory is a poor choice
   * if the file may be very large.
   */
  void read_file_contents( const fc_light::path& filename, std::string& result );

} // namespace fc_light
