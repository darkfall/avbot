#pragma once

#include <string>

#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <boost/avloop.hpp>
#include <boost/filesystem.hpp>
#include <boost/async_dir_walk.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timedcall.hpp>

namespace detail
{

class clean_cache
{
	boost::asio::io_service &io_service;
public:
	clean_cache( boost::asio::io_service &_io_service ): io_service( _io_service )
	{
		// 空闲时执行.
		avloop_idle_post( io_service, *this );
	}

	void operator()()
	{

	}
};

void clean_cache_dir_walk_handler( boost::asio::io_service & io_service, const boost::filesystem::path & item, boost::function<void( const boost::system::error_code& )> handler )
{
	using namespace boost::system;
	handler( error_code() );

	//boost::function<void(const boost::system::error_code&)> h = io_service.wrap(handler);
	if( boost::filesystem::is_regular_file( item ) )
	{
		boost::regex ex( "cache_.*" );
		boost::cmatch what;

		std::string filename  =  boost::filesystem::basename( item );

		if( boost::regex_match( filename.c_str(), what, ex ) )
		{
			boost::posix_time::ptime now = boost::posix_time::from_time_t( std::time( 0 ) );

			// 执行 glob 匹配,  寻找 cache_*
			// 删除最后访问时间超过一天的文件.

			boost::posix_time::ptime last_write_time
				= boost::posix_time::from_time_t( boost::filesystem::last_write_time( item ) );

			if( (now.date() - last_write_time.date()).days() > 0 )
			{
				// TODO! 应该集中到一个地方一起 remove ! 对吧!
				boost::filesystem::remove( item );
			}
		}
	}
}

}

void clean_cache( boost::asio::io_service &io_service )
{
	boost::async_dir_walk( io_service, boost::filesystem::path( "." ),
						   boost::bind( detail::clean_cache_dir_walk_handler, boost::ref( io_service ), _1, _2 )
						 );

	boost::delayedcallsec(io_service, 10000 , boost::bind(clean_cache, boost::ref(io_service)));
}
