package org.magnum.dataup;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

import javax.servlet.http.HttpServletRequest;

import org.magnum.dataup.model.Video;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

@Controller
public class VideoUpController {
	private static final String VIDEO_PATH = "/video";

	private static final AtomicLong currentId = new AtomicLong(0L);
	private Map<Long, Video> videos = new HashMap<Long, Video>();
	
	/**
	 * Saves a video into the server. 
	 * @param v video to save.
	 * @return The video with an unique identified and the URL assigned by the server. 
	 */
	private Video save(Video v) {
		checkAndSet(v);
		videos.put(v.getId(), v);
		return v;
	}

	/**
	 * Set the unique identifier and data URL for the video if were not previously defined.
	 * @param v video
	 */
	private void checkAndSet(Video v) {
		if (v.getId() == 0) {
			// Assign an unique ID 
			v.setId(currentId.incrementAndGet());
			// Assign the data URL
			v.setLocation(getDataUrl(v.getId()));
		}
	}

	/**
	 * Generates the URL of the binary data for the video. 
	 * @param videoId unique identifier for the video.
	 * @return full URL for the video data.
	 */
	private String getDataUrl(long videoId) {
		String url = getUrlBaseForLocalServer() + VIDEO_PATH + videoId
				+ "/data";
		return url;
	}
	/**
	 * Retrieves the URL base of the server.
	 * @return URL base
	 */
	private String getUrlBaseForLocalServer() {
		HttpServletRequest request = ((ServletRequestAttributes) RequestContextHolder
				.getRequestAttributes()).getRequest();
		
		String base = "http://"
				+ request.getServerName()
				+ ((request.getServerPort() != 80) ? ":" + request.getServerPort() : "");
		return base;
	}

	/**
	 * Returns the list of videos that have been added to the server.
	 * @return list of videos
	 */
	@RequestMapping(value = VIDEO_PATH, method = RequestMethod.GET)
	public @ResponseBody List<Video> getVideoList() {

		return new ArrayList<Video>(videos.values());
	}

	/**
	 * Get a video with a specific identifier.
	 * @param videoId unique ID  
	 * @return video object
	 */
	@RequestMapping(value=VIDEO_PATH + "/{videoId}", method=RequestMethod.GET)	
	public Video getVideoById(@PathVariable("videoId") Long videoId){
		if (videos.containsKey(videoId)) {
			return videos.get(videoId);
		} else {
			return null;
		}
	}
	
	/**
	 * Adds a video.
	 * @param v video to add
	 * @return  the video object that was stored along with any updates to that object made by the server.<br>
	 *  The returned Video should include this server-generated identifier and a "data url"
	 */
	@RequestMapping(value=VIDEO_PATH, method=RequestMethod.PUT)
	public @ResponseBody Video addVideo(Video v) {
		save(v);

		return v;
	}
	
	
	


}
