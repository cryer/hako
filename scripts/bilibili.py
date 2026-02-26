import requests
import json
import re
import os
import subprocess
from tqdm import tqdm


def check_ffmpeg():
    try:
        subprocess.run(
            ["ffmpeg", "-version"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True,
        )
        print("✅ ffmpeg 可用")
    except (FileNotFoundError, subprocess.CalledProcessError):
        print(
            "❌ 错误: 未找到 ffmpeg。请先安装 ffmpeg 并确保它已添加到系统 PATH 环境变量中。"
        )
        print("安装指南: https://www.ffmpeg.org/download.html")
        exit(1)


def sanitize_filename(filename):
    # 移除 \ / : * ? " < > | 等非法字符，替换为下划线
    cleaned = re.sub(r'[\\/*?:"<>|]', "_", filename)
    # 移除首尾空格和点
    cleaned = cleaned.strip().strip(".")
    # 限制长度，避免过长文件名
    if len(cleaned) > 100:
        cleaned = cleaned[:100]
    return cleaned


def getResponse(url):
    headers = {
        "Referer": "https://www.bilibili.com/",
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36",
        "Origin": "https://www.bilibili.com",  # 关键！用于音视频流防盗链
    }
    try:
        response = requests.get(url=url, headers=headers, timeout=30)
        response.raise_for_status()  # 如果状态码不是200，会抛出异常
        return response
    except requests.RequestException as e:
        raise Exception(f"网络请求失败: {e}")


def parseResponse(url):
    print("正在解析视频页面...")
    response = getResponse(url)

    # --- 提取视频播放信息 (window.__playinfo__) ---
    playinfo_pattern = r"window\.__playinfo__=(\{.*?\})</script>"
    playinfo_match = re.search(playinfo_pattern, response.text, re.DOTALL)
    if not playinfo_match:
        raise Exception(
            "❌ 解析失败: 未在页面中找到视频播放信息 (window.__playinfo__)。可能是页面结构已更新或视频为番剧/会员专享。"
        )

    try:
        playinfo_json_str = playinfo_match.group(1)
        playinfo_data = json.loads(playinfo_json_str)
    except json.JSONDecodeError:
        raise Exception("❌ 解析失败: 视频播放信息JSON格式错误。")

    # --- 提取视频标题 ---
    # 方法1: 从 <h1> 标签提取
    title_pattern = r'<h1[^>]*?title=["\']([^"\']*)["\']'
    title_match = re.search(title_pattern, response.text)
    if title_match:
        video_title = title_match.group(1).strip()
    else:
        # 方法2: 从 __playinfo__ 的备用字段提取
        video_title = (
            playinfo_data.get("data", {})
            .get("videoData", {})
            .get("title", "未命名视频")
        )
        if video_title == "未命名视频":
            raise Exception("❌ 解析失败: 无法提取视频标题。")

    dash_data = playinfo_data.get("data", {}).get("dash", {})
    audio_list = dash_data.get("audio", [])
    video_list = dash_data.get("video", [])

    if not audio_list:
        raise Exception("❌ 解析失败: 未找到可用的音频流。")
    if not video_list:
        raise Exception("❌ 解析失败: 未找到可用的视频流。")

    # 默认取第一个（通常是最高或默认清晰度）
    audio_url = audio_list[0].get("baseUrl")
    video_url = video_list[0].get("baseUrl")

    if not audio_url:
        raise Exception("❌ 解析失败: 音频URL缺失。")
    if not video_url:
        raise Exception("❌ 解析失败: 视频URL缺失。")

    # 清洗文件名
    safe_title = sanitize_filename(video_title)

    video_info = {
        "videoTitle": safe_title,
        "audioUrl": audio_url,
        "videoUrl": video_url,
    }

    print(f"✅ 解析成功！视频标题: {video_title}")
    return video_info


def saveMedia(file_path, url):
    """
    流式下载媒体文件到指定路径，并显示 tqdm 进度条
    """
    # 创建目录（如果不存在）
    os.makedirs(os.path.dirname(file_path), exist_ok=True)

    headers = {
        "Referer": "https://www.bilibili.com/",
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36",
        "Origin": "https://www.bilibili.com",
    }

    # 发起流式请求
    with requests.get(url, headers=headers, stream=True) as response:
        response.raise_for_status()
        total_size = int(response.headers.get("content-length", 0))

        # 打开文件 + tqdm 进度条
        with (
            open(file_path, "wb") as f,
            tqdm(
                total=total_size,
                unit="B",
                unit_scale=True,
                unit_divisor=1024,
                desc=f"📥 保存 {os.path.basename(file_path)}",
                ncols=80,
                leave=True,
            ) as pbar,
        ):
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:
                    f.write(chunk)
                    pbar.update(len(chunk))

    print(f"✅ 文件已保存: {file_path}")


def AvMerge(mp3_path, mp4_path, output_path):
    print("🎬 开始合并音频和视频...")
    print(f"  音频: {mp3_path}")
    print(f"  视频: {mp4_path}")
    print(f"  输出: {output_path}")

    try:
        # 第一步：获取视频总时长（秒）
        cmd_probe = [
            "ffprobe",
            "-v",
            "error",
            "-show_entries",
            "format=duration",
            "-of",
            "default=noprint_wrappers=1:nokey=1",
            mp4_path,
        ]
        result_probe = subprocess.run(
            cmd_probe, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        if result_probe.returncode != 0:
            raise Exception("无法获取视频时长")

        total_duration = float(result_probe.stdout.strip())
        if total_duration <= 0:
            raise Exception("视频时长无效")

        # 第二步：启动 ffmpeg 合并，并实时读取进度
        cmd = [
            "ffmpeg",
            "-i",
            mp4_path,
            "-i",
            mp3_path,
            "-c:v",
            "copy",
            "-c:a",
            "aac",
            "-strict",
            "experimental",
            "-y",
            output_path,
        ]

        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",  # ← 显式指定编码
            errors="ignore",  # ← 忽略无法解码的字符
            bufsize=1,
            universal_newlines=True,
        )

        # 正则表达式匹配 ffmpeg 输出中的时间
        time_pattern = re.compile(r"time=(\d+):(\d+):(\d+.\d+)")

        # 初始化 tqdm 进度条
        with tqdm(
            total=total_duration, unit="s", desc="🎥 合并进度", ncols=80, leave=True
        ) as pbar:
            current_time = 0.0
            for line in process.stdout:
                match = time_pattern.search(line)
                if match:
                    hours, minutes, seconds = map(float, match.groups())
                    current_time = hours * 3600 + minutes * 60 + seconds
                    # 避免进度回退或超限
                    if current_time > pbar.n:
                        pbar.update(current_time - pbar.n)

        # 等待进程结束
        process.wait()

        if process.returncode != 0:
            print("❌ 合并失败！ffmpeg错误信息:")
            return False
        else:
            print(f"✅ 合并成功！文件已生成: {output_path}")
            # 删除临时文件
            os.remove(mp3_path)
            os.remove(mp4_path)
            print("🗑️  临时的音频、视频文件已删除。")
            return True

    except Exception as e:
        print(f"❌ 合并过程发生异常: {e}")
        return False


def main():
    print("=" * 50)
    print("  🚀 Bilibili 视频下载器")
    print("=" * 50)

    # 1. 检查 ffmpeg
    check_ffmpeg()

    # 2. 获取用户输入
    url = input(
        "\n🔗 请输入B站视频完整URL地址 (例如: https://www.bilibili.com/video/BV1xx411c7Xg): "
    ).strip()
    if not url:
        print("❌ URL不能为空。")
        return

    try:
        # 3. 解析视频信息
        video_info = parseResponse(url)

        # 4. 设置文件路径
        base_dir = "F:\\bilibili"
        file_name = video_info["videoTitle"]
        mp3_path = os.path.join(base_dir, f"{file_name}.mp3")
        mp4_path = os.path.join(base_dir, f"{file_name}.mp4")
        output_path = os.path.join(base_dir, f"【合并完成】{file_name}.mp4")

        # 5. 下载音频
        print("\n⬇️  正在下载音频...")
        saveMedia(mp3_path, video_info["audioUrl"])

        # 6. 下载视频
        print("\n⬇️  正在下载视频...")
        saveMedia(mp4_path, video_info["videoUrl"])

        # 7. 合并音视频
        print("")
        success = AvMerge(mp3_path, mp4_path, output_path)

        if success:
            print("\n🎉 恭喜！视频已成功下载并合并！")
            print(f"📁 保存位置: {output_path}")
        else:
            print("\n⚠️  合并失败，但音频和视频文件已保存，您可以手动处理。")
            print(f"   音频: {mp3_path}")
            print(f"   视频: {mp4_path}")

    except Exception as e:
        print(f"\n❌ 程序执行出错: {e}")
        print("💡 请检查网络、URL是否正确，或尝试其他公开视频。")


if __name__ == "__main__":
    main()
