
package org.magnum.mobilecloud.video.controller;

import java.security.Principal;
import java.util.Collection;
import java.util.Set;

import org.magnum.mobilecloud.video.repository.Video;
import org.magnum.mobilecloud.video.repository.VideoRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import com.google.common.collect.Lists;

/**
 * REST API for Video resources. 
 * 
 * @author Ariel Machado
 */
@Controller
public class VideoUpController {

    private static final String URL_VIDEOS = "/video";
    private static final String URL_VIDEOS_VIDEO = "/video/{id}";
    private static final String URL_VIDEOS_VIDEO_LIKE = "/video/{id}/like";
    private static final String URL_VIDEOS_VIDEO_UNLIKE = "/video/{id}/unlike";
    private static final String URL_VIDEOS_VIDEO_LIKED_BY = "/video/{id}/likedby";
    private static final String URL_VIDEOS_SEARCH_BY_NAME = "/video/search/findByName";
    private static final String URL_VIDEOS_SEARCH_BY_DURATION_LESS = "/video/search/findByDurationLessThan";
    
    
    private static final String PARAMETER_VIDEO_ID = "id";
    private static final String PARAMETER_TITLE = "title";
    private static final String PARAMETER_DURATION = "duration";
    
    @Autowired
    private VideoRepository videoRepository;
    

    /**
     * Returns the list of videos that have been added to the server.
     * 
     * @return list of videos
     */
    @RequestMapping(value = URL_VIDEOS, method = RequestMethod.GET)
    public @ResponseBody ResponseEntity<Collection<Video>> getVideoList() {
    	
    	Collection<Video> videos = Lists.newArrayList(videoRepository.findAll());
    	
    	return new ResponseEntity<Collection<Video>>(videos, HttpStatus.OK);
    }

    /**
     * Get a video with a specific identifier.
     * 
     * @param videoId unique ID
     * @return the video with the given id or {@code 404} if the video is not found.
     */
    @RequestMapping(value = URL_VIDEOS_VIDEO, method = RequestMethod.GET)
    public @ResponseBody ResponseEntity<Video> getVideoById(
    		@PathVariable(PARAMETER_VIDEO_ID) long id) {
    	
		if (!videoRepository.exists(id)) {
			return new ResponseEntity<Video>(HttpStatus.NOT_FOUND);
		} 
		
		Video video = videoRepository.findOne(id); 
		
		return new ResponseEntity<Video>(video, HttpStatus.OK);
    }

    /**
     * Adds a video.
     * 
     * @param v video to add
     * @return the video object that was stored along with any updates to that
     *         object made by the server.<br>
     *         The returned Video should include this server-generated
     *         identifier
     */
    @RequestMapping(value = URL_VIDEOS, method = RequestMethod.POST)
    public @ResponseBody ResponseEntity<Video> addVideo(
    		@RequestBody Video v) {
    	
    	videoRepository.save(v);
    	
    	return new ResponseEntity<Video>(v, HttpStatus.OK);
    }
    
	/**
	 * Allows a user to like a video. 
	 * @return {@code 200 OK} on success, {@code 404 Not Found}  if the video is not found, or  {@code 400 Bad Request} if the user has already liked the video.
	 */
	@RequestMapping(value = URL_VIDEOS_VIDEO_LIKE, method = RequestMethod.POST)
	public ResponseEntity<Void> likeVideo (
			@PathVariable(PARAMETER_VIDEO_ID) long id, Principal p) {
		
		if (!videoRepository.exists(id)){
			return new ResponseEntity<Void>(HttpStatus.NOT_FOUND);
		}
		
		String username = p.getName();
		
		Video v = videoRepository.findOne(id);
		Set<String> likesUsernames = v.getLikesUsernames();
		
		// Checks if the user has already liked the video.
		if (likesUsernames.contains(username)) {
			return new ResponseEntity<Void>(HttpStatus.BAD_REQUEST);
		}
		
		// keep track of users have liked a video
		likesUsernames.add(username);
		v.setLikesUsernames(likesUsernames);
		v.setLikes(likesUsernames.size());
		videoRepository.save(v);
		
		return new ResponseEntity<Void>(HttpStatus.OK);
	}
	
	/**
	 * Allows a user to unlike a video that he/she previously liked.
	 * @param id video identifier
	 * @param p The identity of the currently authenticated user.
	 * @return {@code 200 OK} on success, {@code 404 Not Found} if the video is not found, and a {@code 400 Bad Request} if the user has not previously liked the specified video.
	 */
	@RequestMapping(value = URL_VIDEOS_VIDEO_UNLIKE, method = RequestMethod.POST)
	public ResponseEntity<Void> unlikeVideo(@PathVariable(PARAMETER_VIDEO_ID) long id, Principal p) {

		if (!videoRepository.exists(id)){
			return new ResponseEntity<Void>(HttpStatus.NOT_FOUND);
		}
		
		String username = p.getName();
		
		Video v = videoRepository.findOne(id);
		Set<String> likesUsernames = v.getLikesUsernames();
		
		// Checks if the user has already liked the video.
		if (!likesUsernames.contains(username)) {
			return new ResponseEntity<Void>(HttpStatus.BAD_REQUEST);
		}
		
		// remove track of users have unlike a video
		likesUsernames.remove(username);
		v.setLikesUsernames(likesUsernames);
		v.setLikes(likesUsernames.size());
		videoRepository.save(v);
		
		return new ResponseEntity<Void>(HttpStatus.OK);
	}
	
	/**
	 * The users that have liked the specified video.
	 * @param id video identifier
	 * @return a list of the string usernames of the users that have liked the specified video. If the video is not found, a {@code 4040 Not found} error is generated.
	 */
	@RequestMapping(value = URL_VIDEOS_VIDEO_LIKED_BY, method = RequestMethod.GET)
	public @ResponseBody ResponseEntity<Collection<String>> getUsersWhoLikedVideo (
			@PathVariable(PARAMETER_VIDEO_ID) long id){
		
		if (!videoRepository.exists(id)) {
			return new ResponseEntity<Collection<String>>(HttpStatus.NOT_FOUND);
		}
		
		return new ResponseEntity<Collection<String>>(
				videoRepository.findOne(id).getLikesUsernames(),
				HttpStatus.OK);	
	}
	
	
	/**
	 * Search videos by title.
	 * @param title
	 * @return a list of videos whose titles match the given parameter or an empty list if none are found.
	 */
	@RequestMapping(value = URL_VIDEOS_SEARCH_BY_NAME, method = RequestMethod.GET)
	public @ResponseBody Collection<Video> findByTitle(
			@RequestParam(PARAMETER_TITLE) String title) {
		
		return videoRepository.findByName(title);
	}
	
	/**
	 * Search videos whose durations are less than a input value.
	 * @param duration
	 * @return list of videos whose durations are less than the given parameter or an empty list if none are found.
	 */
	@RequestMapping(value = URL_VIDEOS_SEARCH_BY_DURATION_LESS, method = RequestMethod.GET)
	public @ResponseBody Collection<Video> findByDurationLessThan(
			@RequestParam(PARAMETER_DURATION) long duration) {
		
		return videoRepository.findByDurationLessThan(duration);
	}

}
