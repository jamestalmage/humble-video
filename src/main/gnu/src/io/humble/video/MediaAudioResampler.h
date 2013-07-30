/*******************************************************************************
 * Copyright (c) 2013, Art Clarke.  All rights reserved.
 *  
 * This file is part of Humble-Video.
 *
 * Humble-Video is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Humble-Video is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Humble-Video.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
/*
 * MediaAudioResampler.h
 *
 *  Created on: Jul 30, 2013
 *      Author: aclarke
 */

#ifndef MEDIAAUDIORESAMPLER_H_
#define MEDIAAUDIORESAMPLER_H_

#include <io/humble/video/Configurable.h>
#include <io/humble/video/MediaAudio.h>
extern "C" {
#include <libswresample/swresample.h>
}

namespace io {
namespace humble {
namespace video {

/**
 * A MediaAudioResampler object resamples {@link MediaAudio} objects from
 * one format/sample-rate/channel-layout to another.
 */
class MediaAudioResampler : public io::humble::video::Configurable
{
public:
  static MediaAudioResampler*
  make(
      AudioChannel::Layout outLayout,
      int32_t outSampleRate,
      AudioFormat::Type outFormat,
      AudioChannel::Layout inLayout,
      int32_t inSampleRate,
      AudioFormat::Type inFormat
      );

  virtual AudioChannel::Layout getOutputLayout();
  virtual AudioChannel::Layout getInputLayout();
  virtual int32_t getOutputSampleRate();
  virtual int32_t getInputSampleRate();
  virtual AudioFormat::Type getOutputFormat();
  virtual AudioFormat::Type getInputFormat();
  virtual int32_t getInputChannels();
  virtual int32_t getOutputChannels();

  virtual void open();
  /**
   * Convert audio.
   *
   * in can be set to null to flush the last few samples out at the
   * end.
   *
   * If more input is provided than output space then the input will be buffered.
   * You can avoid this buffering by providing more output space than input.
   * Conversion will run directly without copying whenever possible.
   *
   * @param out       output audio object. caller is responsible for making the correct size.
   * @param in        input audio.
   *
   * @return number of samples output per channel.
   * @throws RuntimeError if we get an error or InvalidArgument if the attributes of
   *   in or out do not match what this resampler expected.
   */
  virtual int32_t resample(MediaAudio* out, MediaAudio* in);

  /**
   * Convert the next timestamp from input to output
   * timestamps are in 1/(in_sample_rate * out_sample_rate) units.
   *
   * @note There are 2 slightly differently behaving modes.
   *       First is when automatic timestamp compensation is not used, (min_compensation >= FLT_MAX)
   *              in this case timestamps will be passed through with delays compensated
   *       Second is when automatic timestamp compensation is used, (min_compensation < FLT_MAX)
   *              in this case the output timestamps will match output sample numbers
   *
   * @param pts   timestamp for the next input sample, INT64_MIN if unknown
   * @return the output timestamp for the next output sample
   */
  virtual int64_t getNextPts(int64_t pts);

  /**
   * Activate resampling compensation.
   */
  virtual void setCompensation(int32_t sample_delta, int32_t compensation_distance);

  /**
   * Set a customized input channel mapping.
   *
   * @param channel_map customized input channel mapping (array of channel
   *                    indexes, -1 for a muted channel)
   * @return AVERROR error code in case of failure.
   */
  //virtual void setChannelMapping(const int *channel_map);

  /**
   * Set a customized remix matrix.
   *
   * @param matrix  remix coefficients; matrix[i + stride * o] is
   *                the weight of input channel i in output channel o
   * @param stride  offset between lines of the matrix
   * @return  AVERROR error code in case of failure.
   */
  //void setMatrix(const double *matrix, int stride);

  /**
   * Drops the specified number of output samples.
   * @return # of samples dropped.
   */
  int32_t dropOutput(int32_t count);

  /**
   * Injects the specified number of silence samples.
   * @return # of samples injected.
   */
  int32_t injectSilence(int32_t count);

  /**
   * Gets the delay the next input sample will experience relative to the next output sample.
   *
   * The resampler can buffer data if more input has been provided than available
   * output space, also converting between sample rates needs a delay.
   * This function returns the sum of all such delays.
   * The exact delay is not necessarily an integer value in either input or
   * output sample rate. Especially when downsampling by a large value, the
   * output sample rate may be a poor choice to represent the delay, similarly
   * for upsampling and the input sample rate.
   *
   * @param base  timebase in which the returned delay will be
   *              if its set to 1 the returned delay is in seconds
   *              if its set to 1000 the returned delay is in milli seconds
   *              if its set to the input sample rate then the returned delay is in input samples
   *              if its set to the output sample rate then the returned delay is in output samples
   *              an exact rounding free delay can be found by using LCM(in_sample_rate, out_sample_rate)
   * @returns     the delay in 1/base units.
   */
  int64_t getDelay(int64_t base);

protected:
  void* getCtx() { return mCtx; }
  MediaAudioResampler();
  virtual
  ~MediaAudioResampler();
private:
  SwrContext* mCtx;
};

} /* namespace video */
} /* namespace humble */
} /* namespace io */
#endif /* MEDIAAUDIORESAMPLER_H_ */