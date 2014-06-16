/******************************************************************************/
#ifndef _MG_NOISE_H_
#define _MG_NOISE_H_
/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//Noise
	/******************************************************************************/
	class Noise3D
	{
	public:
		/* Constructor / Destructor */
		Noise3D(void);
		virtual ~Noise3D(void);

		/* Inititialises the noise function */
		void initialise(unsigned short octaves, double frequency = 1.0, double amplitude = 1.0, double persistence = 1.0);

		/* Returns a noise value between [0, 1]
		@remarks
		The noise is calculated in realtime
		*/
		double noise(double x, double y, double z);

		/* Creates an image file to test the noise */
		//void noise2img(unsigned short dimension = 255);

	protected:
		int p[512];
		unsigned short mOctaves;
		double mFrequency;
		double mAmplitude;
		double mPersistence;

		/* Returns a noise value between [0, 1]
		@remarks
		The noise is calculated in realtime
		*/
		double _noise(double x, double y, double z);

		double _fade(double t);
		double _lerp(double t, double a, double b);
		double _grad(int hash, double x, double y, double z);
	};
}

#endif