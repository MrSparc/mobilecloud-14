package org.magnum.dataup;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.magnum.dataup.model.Video;
import org.magnum.dataup.model.VideoStatus;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;
import org.springframework.web.multipart.MultipartFile;

@Controller
public class VideoUpController {

	private static final String VIDEO_SVC_PATH = "/video";
	private static final String ID_PARAMETER = "id";
	private static final String VIDEO_PATH = VIDEO_SVC_PATH + "/{"
			+ ID_PARAMETER + "}";
	private static final String VIDEO_DATA_PATH = VIDEO_PATH + "/data";
	private static final String DATA_PARAMETER = "data";

	private static final AtomicLong currentId = new AtomicLong(0L);
	private Map<Long, Video> videos = new HashMap<Long, Video>();
	private VideoFileManager videoDataMgr;

	VideoUpController() {
		try {
			videoDataMgr = VideoFileManager.get();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	};

	/**
	 * Saves a video into the server.
	 * 
	 * @param v
	 *            video to save.
	 * @return The video with an unique identified and the URL assigned by the
	 *         server.
	 */
	private Video save(Video v) {
		checkAndSet(v);
		videos.put(v.getId(), v);
		return v;
	}

	/**
	 * Set the unique identifier and data URL for the video if were not
	 * previously defined.
	 * 
	 * @param v
	 *            video
	 */
	private void checkAndSet(Video v) {
		if (v.getId() == 0) {
			// Assign an unique ID
			v.setId(currentId.incrementAndGet());
			// Assign the data URL
			v.setDataUrl(getDataUrl(v.getId()));
		}
	}

	/**
	 * Generates the URL of the binary data for the video.
	 * 
	 * @param videoId
	 *            unique identifier for the video.
	 * @return full URL for the video data.
	 */
	private String getDataUrl(long videoId) {
		String url = getUrlBaseForLocalServer() + VIDEO_SVC_PATH + "/"
				+ videoId + "/data";
		return url;
	}

	/**
	 * Retrieves the URL base of the server.
	 * 
	 * @return URL base
	 */
	private String getUrlBaseForLocalServer() {
		HttpServletRequest request = ((ServletRequestAttributes) RequestContextHolder
				.getRequestAttributes()).getRequest();

		String base = "http://"
				+ request.getServerName()
				+ ((request.getServerPort() != 80) ? ":"
						+ request.getServerPort() : "");
		return base;
	}

	/**
	 * Returns the list of videos that have been added to the server.
	 * 
	 * @return list of videos
	 */
	@RequestMapping(value = VIDEO_SVC_PATH, method = RequestMethod.GET)
	public @ResponseBody Collection<Video> getVideoList() {

		return new ArrayList<Video>(videos.values());
	}

	/**
	 * Get a video with a specific identifier.
	 * 
	 * @param videoId
	 *            unique ID
	 * @return video object or null if doesn't exist
	 */
	@RequestMapping(value = VIDEO_PATH, method = RequestMethod.GET)
	public @ResponseBody Video getVideoById(@PathVariable(ID_PARAMETER) long id) {
		if (videos.containsKey(id)) {
			return videos.get(id);
		} else {
			return null;
		}
	}

	/**
	 * Adds a video.
	 * 
	 * @param v
	 *            video to add
	 * @return the video object that was stored along with any updates to that
	 *         object made by the server.<br>
	 *         The returned Video should include this server-generated
	 *         identifier and a "data url"
	 */
	@RequestMapping(value = VIDEO_SVC_PATH, method = RequestMethod.POST)
	public @ResponseBody Video addVideo(@RequestBody Video v) {
		save(v);
		return v;
	}

	/**
	 * Upload video data 
	 * @param id video identifier
	 * @param videoData multipart data
	 * @return Status
	 */
	@RequestMapping(value = VIDEO_DATA_PATH, method = RequestMethod.POST)
	public @ResponseBody ResponseEntity<VideoStatus> addVideoData(
			@PathVariable(ID_PARAMETER) long id,
			@RequestParam(DATA_PARAMETER) MultipartFile videoData) {

		Video v = getVideoById(id);
		if (v == null) {
			return new ResponseEntity<VideoStatus>(HttpStatus.NOT_FOUND);
		}

		try {
			InputStream in = videoData.getInputStream();
			videoDataMgr.saveVideoData(v, in);
			in.close();
			return new ResponseEntity<VideoStatus>(
					new VideoStatus(VideoStatus.VideoState.READY), 
					HttpStatus.OK);
		} catch (IOException e) {
			return new ResponseEntity<VideoStatus>(
					HttpStatus.INTERNAL_SERVER_ERROR);
		}

	}
	
	/**
	 * Returns the video data in streaming
	 * @param id video identifier
	 * @param response servlet response to write data streaming
	 * @throws IOException 
	 */
	@RequestMapping(value = VIDEO_DATA_PATH, method = RequestMethod.GET) 
	public void getVideoData(
			@PathVariable(ID_PARAMETER) long id, 
			HttpServletResponse response) throws IOException{
		
		Video v = getVideoById(id);
		if (v == null) {
			response.sendError(HttpServletResponse.SC_NOT_FOUND);
		} else {
			response.setContentType(v.getContentType());
			OutputStream out =  response.getOutputStream();
			videoDataMgr.copyVideoData(v, out);
		}
		
	}

}
