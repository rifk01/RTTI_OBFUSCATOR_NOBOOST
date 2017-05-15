#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <time.h>
#include <cstdlib>
#include <regex>
#include <unordered_set>

//#include <boost/regex.hpp>
//#include <boost/filesystem.hpp>

using namespace std;

std::unordered_set<std::string> usedTypeNames;

std::string getRandomString( const int len ) {
    static const char alpha[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    string s;
    s.resize( len );

    for ( auto i = 0; i < len; ++i ) {
        // we always want to start types with an alphabetical letter
        if ( 0 == i )
        {
            s[i] = alpha[rand() % ( sizeof( alpha ) - 1 )];

        }
        else
        {
            s[i] = alphanum[rand() % ( sizeof( alphanum ) - 1 )];
        }
    }

    s[len] = 0;

    return s;
}

// really like that boost regexp callback idea!
namespace std
{
    template<class Iterator, class Settings, class StringType, class SingleArg>
    std::basic_string<StringType> regex_replace( Iterator iterStart, Iterator iterFinish, const std::basic_regex<StringType, 
        Settings>& expressions, SingleArg sArg )
    {
        std::basic_string<StringType> sBuffer;

        typename std::match_results<Iterator>::difference_type lastPos = 0;
        auto lastPosEnd = iterStart;

        auto regexp_callback = [&]( const std::match_results<Iterator>& m )
        {
            auto pos = m.position( 0 );
            auto difference = pos - lastPos;

            auto start = lastPosEnd;
            std::advance( start, difference );

            sBuffer.append( lastPosEnd, start );
            sBuffer.append( sArg( m ) );

            auto len = m.length( 0 );

            lastPos = pos + len;

            lastPosEnd = start;
            std::advance( lastPosEnd, len );
        };

        std::sregex_iterator start( iterStart, iterFinish, expressions ), finish;
        std::for_each( start, finish, regexp_callback );

        sBuffer.append( lastPosEnd, iterFinish );

        return sBuffer;
    }

    template<class Settings, class StringType, class SingleArg>
    std::string regex_replace( const std::string& sBuffer,
        const std::basic_regex<StringType, Settings>& expressions, SingleArg sArg )
    {
        auto begin = sBuffer.cbegin();
        auto end = sBuffer.cend();
        return regex_replace( begin, end, expressions, sArg );
    }
}

int main( int argc, char* argv[] )
{
    cout << "===== RTTI obfuscator by koemeet/rifk01 =====" << endl;

    srand( time( nullptr ) );

    try
    {
        if ( argc < 2 )
        {
            throw std::exception( "Please provide a path or drop a file into me." );
        }

        // path to input binary
        string path = argv[1];

        std::ifstream fs( path, std::fstream::binary );
        if ( fs.fail() )
        {
            throw std::exception( "Could not open source binary" );
        }

        // read file contents
        std::stringstream ss;
        ss << fs.rdbuf();
        auto contents = ss.str();

        std::regex reg( "\\.(\\?AV|PEAV)(.+?)@@" );
        //boost::regex reg( "\\.(\\?AV|PEAV)(.+?)@@" );

        // replace RTTI types
        contents = std::regex_replace( contents, reg, []( const std::smatch& m )
        {
            auto prefix = m[1].str();
            auto typeName = m[2].str();

            // generate the new type name for the current rtti entry
            auto newTypeName = getRandomString( typeName.size() );

            // get a new random name untill we have one we never used before
            while ( usedTypeNames.find( newTypeName ) != usedTypeNames.end() )
            {
                newTypeName = getRandomString( typeName.size() );
            }

            usedTypeNames.emplace( newTypeName );

            return "." + prefix + newTypeName + "@@";
        } );

        // generate output path
        //boost::filesystem::path p( path );
        //auto outputPath = p.parent_path().string() + "\\" + p.stem().string() + ".tan" + p.extension().string();

        string fileExtension = path.substr( path.length() - 3 );
        path.erase( path.length() - 3 );
        // write to file
        path += "tan.";
        path += fileExtension;

        std::ofstream os( path.c_str(), std::ofstream::trunc | std::ofstream::binary );
        if ( !os.write( contents.data(), contents.size() ) )
        {
            throw std::exception( ( std::string( "Unable to write to file " ) + path ).c_str() );
        }
        else
        {
            cout << "Successfully obfuscated RTTI information. Output written to " << path << endl;
            system( "pause" );
        }
    }

    catch ( std::exception &e )
    {
        cout << e.what() << endl;
        system( "pause" );
        return 1;
    }

    return 0;
}
