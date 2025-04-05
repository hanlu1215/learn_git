import re
import os
import subprocess
import cv2
import numpy as np
from tkinter import Tk, filedialog
from concurrent.futures import ThreadPoolExecutor
from moviepy import VideoFileClip, concatenate_videoclips
output_path = f'F:/文献的视频资料/'

def contains_chinese(text):
    """检查文本是否包含中文字符"""
    chinese_pattern = re.compile('[\u4e00-\u9fff]')
    return bool(chinese_pattern.search(text))

def create_black_image_with_text(text, width=1920, height=1080):
    img = np.zeros((height, width, 3), dtype=np.uint8)
    font = cv2.FONT_HERSHEY_SIMPLEX
    font_scale = 2
    font_thickness = 3
    line_spacing = 20  # 每行文字之间的间距（像素）

    # 将文本按照换行符进行分割
    lines = text.split('\n')

    # 处理每一行，按宽度拆分多行
    wrapped_lines = []
    for line in lines:
        current_line = ""
        for word in line.split():
            test_line = current_line + (" " if current_line else "") + word
            text_size = cv2.getTextSize(test_line, font, font_scale, font_thickness)[0]
            if text_size[0] <= width - 40:  # 留边距
                current_line = test_line
            else:
                wrapped_lines.append(current_line)
                current_line = word
        if current_line:  # 加入最后未保存的一行
            wrapped_lines.append(current_line)

    # 计算文字开始绘制的初始 y 坐标，使文字垂直居中
    total_text_height = len(wrapped_lines) * (cv2.getTextSize("Test", font, font_scale, font_thickness)[0][1] + line_spacing)
    start_y = (height - total_text_height) // 2

    # 绘制每行文字
    y_offset = start_y
    for line in wrapped_lines:
        text_size = cv2.getTextSize(line, font, font_scale, font_thickness)[0]
        text_x = (width - text_size[0]) // 2
        text_y = y_offset + text_size[1]
        cv2.putText(img, line, (text_x, text_y), font, font_scale, (255, 255, 255), font_thickness)
        y_offset += text_size[1] + line_spacing

    return img


def process_video_file(i, video_file, temp_folder, create_video):
    final_videos = []
    try:
        if create_video:
            # Step 1: Create black image and add text
            image_with_text = create_black_image_with_text(video_file)
            temp_image_file = os.path.join(temp_folder, f"temp_image_{i}.jpg")
            cv2.imwrite(temp_image_file, image_with_text)
            temp_image_video_file = os.path.join(temp_folder, f"temp_image_video_{i}.mp4")
            
            command_image_video = [
                "ffmpeg", "-loop", "1", "-framerate", "1", "-t", "1",
                "-i", temp_image_file,
                "-f", "lavfi", "-t", "1", "-i", "anullsrc=r=44100:cl=stereo",
                "-c:v", "libx264", "-pix_fmt", "yuv420p", "-r", "30",
                temp_image_video_file
            ]
            subprocess.run(command_image_video, check=True)
            final_videos.append(temp_image_video_file)

        # Step 2: Scale video to 1080p and change framerate
        scaled_file = os.path.join(temp_folder, f"temp_scaled_{i}.mp4")
        command_scale = [
            "ffmpeg", "-i", video_file,
            "-vf", "scale=w=1920:h=1080:force_original_aspect_ratio=decrease,pad=1920:1080:(1920-iw)/2:(1080-ih)/2",
            "-r", "30",
            "-c:v", "libx264",
            "-preset", "fast",
            "-crf", "23",
            "-c:a", "aac",
            scaled_file
        ]
        subprocess.run(command_scale, check=True)
        final_videos.append(scaled_file)
        
    except subprocess.CalledProcessError as e:
        print(f"处理视频 {video_file} 时出错: {e}")

    return final_videos

def merge_videos_in_current_path(folder_name,output_path):
    video_files = [
        f for f in sorted(os.listdir('.'))
        if os.path.isfile(f)
        and f.lower().endswith(('.mp4', '.avi', '.mov', '.mkv'))
        and not f.endswith('-merge.mp4')
    ]
    # 优先将包含 "main" 的文件提取出来
    videos_with_main = [f for f in video_files if 'main' in f.lower()]
    videos_without_main = [f for f in video_files if 'main' not in f.lower()]
    # 合并列表，优先显示包含 "main" 的视频
    video_files = videos_with_main + videos_without_main
    if not video_files:
        print("当前路径下没有找到有效的视频文件。")
        return
    temp_folder = os.path.join('.', 'temp_folder')
    if os.path.exists(temp_folder):
            for temp_file in os.listdir(temp_folder):
                os.remove(os.path.join(temp_folder, temp_file))
            os.rmdir(temp_folder)
    if not os.path.exists(temp_folder):
        os.makedirs(temp_folder)
    output_file = os.path.join(output_path, f"{folder_name}-merge.mp4")
    # If output file exists, delete it
    if os.path.exists(output_file):
        os.remove(output_file)

    # Use an existing local video as the disclaimer video
    existing_disclaimer_video = "F:/文献的视频资料/已发/disclaimer_video.mp4"
    if not os.path.exists(existing_disclaimer_video):
        print(f"指定的免责声明视频文件不存在: {existing_disclaimer_video}")
            # Create a disclaimer image with the required text
        disclaimer_text = f'For educational purposes only, not for commercial use.\nIf there is any infringement, please contact for removal.'
        disclaimer_image = create_black_image_with_text(disclaimer_text)
        disclaimer_image_file = os.path.join(temp_folder, "disclaimer_image.jpg")
        cv2.imwrite(disclaimer_image_file, disclaimer_image)
        temp_disclaimer_video_file = os.path.join(temp_folder, "disclaimer_image_video.mp4")

        # Generate the disclaimer video (1 second duration)
        command_disclaimer_video = [
            "ffmpeg", "-loop", "1", "-framerate", "1", "-t", "3",  # 3 seconds for visibility
            "-i", disclaimer_image_file,
            "-f", "lavfi", "-t", "3", "-i", "anullsrc=r=44100:cl=stereo",
            "-c:v", "libx264", "-pix_fmt", "yuv420p", "-r", "30",
            temp_disclaimer_video_file
        ]
        subprocess.run(command_disclaimer_video, check=True)
    else:
        temp_disclaimer_video_file = existing_disclaimer_video
    # Create a thread pool to parallelize the processing of videos
    with ThreadPoolExecutor() as executor:
        final_videos = []
        
        for i, video_file in enumerate(video_files):
            create_video = not contains_chinese(video_file)  # Skip creating video for Chinese text files
            future = executor.submit(process_video_file, i, video_file, temp_folder, create_video)
            final_videos.extend(future.result())
        
        # Step 3: Merge all videos including disclaimer image video at the end
        final_videos.append(temp_disclaimer_video_file)  # Add the disclaimer video at the end

        temp_list_file = os.path.join(temp_folder, "temp_file_list.txt")
        with open(temp_list_file, "w") as f:
            for video_file in final_videos:
                f.write(f"file '{video_file}'\n")
        # 加载视频文件
        clips = [VideoFileClip(video_file) for video_file in final_videos]
        # 合并视频
        final_clip = concatenate_videoclips(clips, method="compose")
        # 保存合并后的视频
        final_clip.write_videofile(output_file, codec="libx264", fps=30)

        print(f"视频已成功合并，并保存为: {output_file}")
        #Clean up
        if os.path.exists(temp_folder):
            for temp_file in os.listdir(temp_folder):
                os.remove(os.path.join(temp_folder, temp_file))
            os.rmdir(temp_folder)

# Main entry
if __name__ == "__main__":
    Tk().withdraw()  # Hide the root window
    folder_path = filedialog.askdirectory(title="选择包含视频文件的文件夹")
    if not folder_path:
        print("未选择文件夹。")
    else:
        os.chdir(folder_path)
        folder_name = os.path.basename(folder_path)
        merge_videos_in_current_path(folder_name,output_path)
    os.system('pause')