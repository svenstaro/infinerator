#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#elif __WINDOWS__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#include <math.h>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>
#include <cassert>

#include <boost/multi_array.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#define foreach BOOST_FOREACH

namespace po = boost::program_options;
using namespace std;

typedef boost::multi_array<Uint16, 2> array_type;

bool isSize (string size_str) {
	// verify correct format
	boost::regex expression("^[1-9][0-9]*x[1-9][0-9]*$");
	boost::cmatch matches;
	if (regex_match(size_str.c_str(), matches, expression))
		return true;
	else
		return false;
}

array_type makeColorList (string s) {
	// boost ndim array to hold our colorlist
	if (s == "bw") {
		array_type cl(boost::extents[2][3]);
		for(array_type::index i = 0; i < 3; ++i) {
			cl[0][i] = 0;
			cl[1][i] = 255;
		}
		return cl;
	}
	else if (s == "grey") {
		Uint16 c = 0;
		array_type cl(boost::extents[5][3]);
		for(array_type::index i = 0; i < 5; i++) {
			for(array_type::index j = 0; j < 3; ++j) {
				cl[i][j] = c;
			}
			c += 51;
		}
		return cl;
	}
	else if (s == "rgby") {
		array_type cl(boost::extents[4][3]);
		cl[0][0] = 255;
		cl[1][1] = 255;
		cl[2][2] = 255;
		cl[3][0] = 255;
		cl[3][1] = 255;
		return cl;
	}
	else if (s == "16") {
		Uint16 c = 0;
		array_type cl(boost::extents[16][3]);
		for(array_type::index i = 0; i < 16; i++) {
			for(array_type::index j = 0; j < 3; ++j) {
				cl[i][j] = c;
			}
			c += 16;
		}
		return cl;
	}
	else if (s == "256") {
		Uint16 c = 0;
		array_type cl(boost::extents[256][3]);
		for(array_type::index i = 0; i < 256; i++) {
			for(array_type::index j = 0; j < 3; ++j) {
				cl[i][j] = c;
			}
			c += 1;
		}
		return cl;
	}
}

void printArray (array_type& cm, Uint16& h, Uint16& w) {
	// print out array for debugging
	Uint16 ix = 0; Uint16 iy = 0;
	while (iy < h) {
		if (ix == w) {
			iy++;
			ix = 0;
			cout << endl;
		}
		if (iy < h) {
			cout << cm[ix][iy];
			ix++;
		}
	}
	cout << endl;
}

vector<Uint16> splitSize (string size_str) {
	// split "4x4" to int{4,4}
	vector<Uint16> size_vec;
	Uint16 sep;
	string width,height;

	sep = size_str.find("x");

	width = size_str.substr(0,sep);
	height = size_str.substr(sep+1);

	size_vec.push_back(boost::lexical_cast<Uint16>(width));
	size_vec.push_back(boost::lexical_cast<Uint16>(height));

	return size_vec;
}

void getNext (array_type& colormap, const array_type& colorlist, const Uint16& height, const Uint16& width, Uint16& x,Uint16& y) {
	// figures out the next block to fill color in
	// pass the current value of x and y into here
	// getNext will pass back the new x and y by reference

	// condition for incrementing selected item
	if ( colormap[x][y] < colorlist.size()-1 ) {
		colormap[x][y]++;
	}
	// condition for vertical move (y)
	else if ( colormap[x][y] == colorlist.size()-1 && y < height-1 ) {
		colormap[x][y] = 0;
		if ( colormap[x][y+1] == colorlist.size()-1 ) {
			y++;
			getNext(colormap, colorlist, height, width, x, y);
		}
		else {
			colormap[x][y+1] += 1;
			y++;
		}
	}
	// condition for horizontal move (x)
	else if ( colormap[x][y] == colorlist.size()-1 && y == height-1 ) {
		colormap[x][y] = 0;
		x++; y = 0;
		getNext(colormap, colorlist, height, width, x, y);
	}
}

po::variables_map usage ( int& ac, char** av ) {
	try {
		// Declare the supported options.
		po::options_description desc("This is a program to generate ALL possible images of a given color for a given resolution.\
		\n\nAllowed options");
		desc.add_options()
			("help", "this message, obviously")
			("colorset,c", po::value<string>()->default_value("grey"), "choose colorset <colorsets: 'bw', 'grey', 'rgby', '16', '256'>")
			("size,s", po::value<string>()->default_value("5x4"), "size of generated images in pixels <format: WxH>")
			("mode,m", po::value<string>()->default_value("display"), "valid values for mode are: 'display', 'save', 'both'")
			("outdir,o", po::value<string>()->default_value("./out"), "outdir for generated images if mode is 'save' or 'both'")
			("interactive,i", po::value<Uint16>()->implicit_value(1), "enable interactive mode (press return for each image)")
			("every,e", po::value<Uint16>()->default_value(1000), "display every nth image only (desirable for performance reasons)")
			("dotsize,d", po::value<Uint16>()->default_value(50), "dot (pixel) size")
			("verbose,v", po::value<Uint16>()->implicit_value(1), "be verbose")
			("quiet,q", po::value<Uint16>()->implicit_value(1), "be quiet")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(ac, av, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			cout << desc << "\n";
			exit(1);
		}

		if (isSize(vm["size"].as<string>())) {
			cout << "Size is " << vm["size"].as<string>() << ".\n";
		} else {
			cerr << "ERROR: Wrong image size format.\n";
			exit(1);
		}

		return vm;

	}
    catch(exception& e) {
		cerr << "ERROR: " << e.what() << "\n";
        exit(1);
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }
}


int main ( int argc, char** argv )
{
	po::variables_map opts = usage ( argc, argv );

	// 2d container for our colorlist (colors are RGB pairs)
	array_type colorlist = makeColorList(opts["colorset"].as<string>());
	Uint16 width = splitSize(opts["size"].as<string>())[0];
	Uint16 height = splitSize(opts["size"].as<string>())[1];
	string mode = opts["mode"].as<string>();
	string outdir = opts["outdir"].as<string>();
	//Uint16 interactive = opts["interactive"].as<Uint16>();
	Uint16 every = opts["every"].as<Uint16>();
	Uint16 dotsize = opts["dotsize"].as<Uint16>();
	//Uint16 verbose = opts["verbose"].as<Uint16>();
	//Uint16 quiet = opts["quiet"].as<Uint16>();

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

	// make sure new window is centered
	SDL_putenv("SDL_VIDEO_CENTERED=center");

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(width*dotsize, height*dotsize, 16,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    };

	// separate color map because it's easier than comparing pixel RGB values
	// filled with colorlist value 0
	// might look like this (for black and white):
	// 	0 0 0	1 0 0	0 0 0	1 0 0	0 0 0
	// 	0 0 0 >	0 0 0 >	1 0 0 > 1 0 0 > 0 0 0
	// 	0 0 0	0 0 0	0 0 0	0 0 0	1 0 0
	array_type colormap(boost::extents[width][height]);
	for(array_type::index i = 0; i != width; ++i)
		for(array_type::index j = 0; j != height; ++j)
			colormap[i][j] = 0;

	// calculate how many possible combinations there are
	Uint32 nperm = pow(colorlist.size(), (width*height));

	// clear screen
	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

    // program main loop
    bool done = false;
    while (!done) {

		SDL_Event event;
		Uint16 x = 0;
		Uint16 y = 0;
		SDL_Rect lolrect;

		for (Uint32 n = 0; n < nperm-1; ++n) {

			if (opts.count("interactive")) {
				while (SDL_WaitEvent(&event) >= 0) {
					switch (event.type) {
						case SDL_KEYDOWN:
							cout << "lol";
						break;
					}
				}
			}

			// message processing loop
			while (SDL_PollEvent(&event)) {
				// check for messages
				switch (event.type) {
					// exit if the window is closed
					case SDL_QUIT: {
						done = true;
						break;
					}
						// check for keypresses
					case SDL_KEYDOWN: {
						// exit if ESCAPE is pressed
						if (event.key.keysym.sym == SDLK_ESCAPE)
							done = true;
						break;
					}
				} // end switch
			} // end of message processing

			// DRAWING STARTS HERE

			lolrect.w = dotsize; lolrect.h = dotsize;
			// reset these so the cursor starts at upper left corner
			x = 0; y = 0;
			getNext(colormap, colorlist, height, width, x, y);

			if (opts.count("verbose")) {
				// print the whole colormap for debugging purposes as text
				printArray(colormap, height, width);
			}

			if (n % every == 0) {
				// actual display is happening here
				for (Uint16 lx = 0; lx < width; lx++) {
					for (Uint16 ly = 0; ly < height; ly++) {
						lolrect.x = lx*dotsize; lolrect.y = ly*dotsize;
						SDL_FillRect(screen, &lolrect, SDL_MapRGB(screen->format, colorlist[colormap[lx][ly]][0], colorlist[colormap[lx][ly]][1], colorlist[colormap[lx][ly]][2]));
					}
				}
				// flip only when done drawing a whole image
				SDL_Flip(screen);

				// report status
				if (!opts.count("quiet")) {
					cout << "Finished " << n << " images" << endl;
				}
			}
		}

        // DRAWING ENDS HERE

        // finally, update the screen once more :)
        SDL_Flip(screen);

        done = true;
    } // end main loop

    // all is well ;)
	cout << "Finished at " << nperm << " images" << endl;
    if (opts.count("verbose")) {
		cout << "Exited cleanly :)" << endl;
    }

    return 0;
}
