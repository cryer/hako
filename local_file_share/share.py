import os
import sys
import socket
import uuid
import shutil
import tempfile
import threading
import time
import webbrowser
import zipfile
import io
import base64
from pathlib import Path

try:
    from flask import Flask, request, render_template_string, send_file, jsonify, send_from_directory
    import qrcode
except ImportError:
    print("❌ 缺少依赖库！")
    print("请打开 CMD 运行: pip install flask qrcode[pil]")
    sys.exit(1)

app = Flask(__name__)
PORT = 8888  
BASE_TEMP_DIR = Path(tempfile.gettempdir()) / "lan_share_tool"

if BASE_TEMP_DIR.exists():
    try:
        shutil.rmtree(BASE_TEMP_DIR)
    except:
        pass
BASE_TEMP_DIR.mkdir(parents=True, exist_ok=True)

ROOMS = {}


def get_real_lan_ip():
    candidates = []
    try:
        hostname = socket.gethostname()
        _, _, ip_list = socket.gethostbyname_ex(hostname)
        
        for ip in ip_list:
            if ip.startswith("127."): 
                continue
            if ip.startswith("198.18."):
                continue
            if ip.startswith("172.17."):
                continue

            candidates.append(ip)

    except Exception as e:
        print(f"IP 获取错误: {e}")
        return "127.0.0.1"

    if not candidates:
        return "127.0.0.1"

    for ip in candidates:
        if ip.startswith("192.168."):
            return ip
            
    for ip in candidates:
        if ip.startswith("10."):
            return ip
            
    for ip in candidates:
        if ip.startswith("172."):
            return ip

    return candidates[0]


def generate_qr_base64(data):
    qr = qrcode.QRCode(version=1, box_size=10, border=2)
    qr.add_data(data)
    qr.make(fit=True)
    img = qr.make_image(fill_color="black", back_color="white")
    buffer = io.BytesIO()
    img.save(buffer, format="PNG")
    img_str = base64.b64encode(buffer.getvalue()).decode()
    return f"data:image/png;base64,{img_str}"


def format_size(size):
    power = 2**10
    n = 0
    power_labels = {0 : '', 1: 'K', 2: 'M', 3: 'G', 4: 'T'}
    while size > power:
        size /= power
        n += 1
    return f"{size:.2f} {power_labels[n]}B"


HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>快传 (Clash Compatible)</title>
    <style>
        :root { --primary: #007bff; --bg: #f4f6f9; }
        body { font-family: -apple-system, sans-serif; background: var(--bg); margin: 0; padding: 20px; color: #333; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 12px; box-shadow: 0 4px 20px rgba(0,0,0,0.08); }
        h1 { text-align: center; color: #2c3e50; margin-bottom: 5px; }
        .ip-info { text-align: center; font-size: 0.85em; color: #666; background: #eee; padding: 4px 10px; border-radius: 20px; display: inline-block; margin-bottom: 20px;}
        
        /* 上传组件 */
        .upload-box { border: 2px dashed #bdc3c7; border-radius: 10px; padding: 40px; text-align: center; cursor: pointer; background: #fafafa; transition: 0.2s; }
        .upload-box:hover { border-color: var(--primary); background: #eef7ff; }
        
        /* 文件列表 */
        .file-list { margin-top: 20px; }
        .file-item { display: flex; justify-content: space-between; align-items: center; padding: 12px; border-bottom: 1px solid #eee; }
        .btn { padding: 8px 16px; background: var(--primary); color: white; text-decoration: none; border-radius: 6px; border: none; cursor: pointer; font-size: 0.9em; }
        .btn:hover { background: #0056b3; }
        .btn-success { background: #28a745; display: block; text-align: center; margin-top: 15px; width: 100%; box-sizing: border-box; padding: 12px; }
        
        /* 二维码 */
        .qr-area { text-align: center; margin-top: 20px; border-top: 1px solid #eee; padding-top: 20px; display: none; }
        .qr-area.active { display: block; animation: fadeUp 0.3s; }
        .qr-img { max-width: 220px; border: 1px solid #ddd; padding: 5px; border-radius: 8px; }
        .url-text { font-family: monospace; background: #f0f0f0; padding: 8px; border-radius: 4px; word-break: break-all; margin-top: 10px; font-size: 0.9em;}
        
        @keyframes fadeUp { from { opacity: 0; transform: translateY(10px); } to { opacity: 1; transform: translateY(0); } }
    </style>
</head>
<body>

<div class="container">
    <div style="text-align: center;">
        <h1>🚀 局域网快传</h1>
        <div class="ip-info">Host IP: {{ host_ip }}</div>
    </div>

    {% if mode == 'upload' %}
        <div class="upload-box" id="dropZone" onclick="document.getElementById('fileInput').click()">
            <div style="font-size: 48px;">📁</div>
            <h3>点击或拖拽文件上传</h3>
            <div style="color: #999; font-size: 0.9em;">支持多文件</div>
        </div>
        <input type="file" id="fileInput" multiple style="display:none">

        <div class="qr-area" id="qrArea">
            <h3>📱 手机扫码下载</h3>
            <img id="qrImage" class="qr-img" src="">
            <div class="url-text" id="shareUrl"></div>
            <button class="btn" onclick="location.reload()" style="margin-top: 15px; background: #6c757d;">发送新文件</button>
        </div>

    {% else %}
        <div class="file-list">
            {% for file in files %}
            <div class="file-item">
                <div style="flex:1; padding-right: 10px;">
                    <div style="font-weight: 500;">{{ file.name }}</div>
                    <div style="font-size: 0.8em; color: #888;">{{ file.size_str }}</div>
                </div>
                <a href="/download/{{ room_id }}/{{ file.name }}" class="btn" download>下载</a>
            </div>
            {% endfor %}
        </div>

        {% if files|length > 1 %}
        <a href="/zip/{{ room_id }}" class="btn btn-success">📦 打包下载全部 (ZIP)</a>
        {% endif %}
    {% endif %}
</div>

{% if mode == 'upload' %}
<script>
    const dropZone = document.getElementById('dropZone');
    const qrArea = document.getElementById('qrArea');

    // 拖拽处理
    ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(e => {
        dropZone.addEventListener(e, ev => { ev.preventDefault(); ev.stopPropagation(); });
    });
    dropZone.addEventListener('drop', e => handleFiles(e.dataTransfer.files));
    document.getElementById('fileInput').addEventListener('change', e => handleFiles(e.target.files));

    function handleFiles(files) {
        if (!files.length) return;
        const formData = new FormData();
        for (let i = 0; i < files.length; i++) formData.append('files', files[i]);

        dropZone.innerHTML = '<h3>⏳ 上传处理中...</h3>';
        
        fetch('/upload', { method: 'POST', body: formData })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    dropZone.style.display = 'none';
                    qrArea.classList.add('active');
                    document.getElementById('qrImage').src = data.qr_code;
                    document.getElementById('shareUrl').innerText = data.url;
                } else {
                    alert('上传失败: ' + data.msg);
                    location.reload();
                }
            })
            .catch(err => { alert('网络错误'); location.reload(); });
    }
</script>
{% endif %}
</body>
</html>
"""


@app.route('/')
def index():
    return render_template_string(HTML_TEMPLATE, mode='upload', host_ip=get_real_lan_ip())


@app.route('/upload', methods=['POST'])
def upload():
    files = request.files.getlist('files')
    if not files or files[0].filename == '':
        return jsonify({'success': False, 'msg': '未选择文件'})

    room_id = str(uuid.uuid4())[:6]
    room_path = BASE_TEMP_DIR / room_id
    room_path.mkdir(exist_ok=True)
    
    saved_files = []
    for file in files:
        filename = os.path.basename(file.filename)
        save_path = room_path / filename
        file.save(save_path)
        saved_files.append({
            'name': filename,
            'size_str': format_size(save_path.stat().st_size)
        })

    ROOMS[room_id] = {'path': room_path, 'files': saved_files}

    real_ip = get_real_lan_ip()
    share_url = f"http://{real_ip}:{PORT}/s/{room_id}"
    
    return jsonify({
        'success': True,
        'url': share_url,
        'qr_code': generate_qr_base64(share_url)
    })


@app.route('/s/<room_id>')
def share_page(room_id):
    if room_id not in ROOMS: return "页面已过期", 404
    return render_template_string(
        HTML_TEMPLATE, 
        mode='download', 
        files=ROOMS[room_id]['files'], 
        room_id=room_id,
        host_ip=get_real_lan_ip()
    )


@app.route('/download/<room_id>/<filename>')
def download_file(room_id, filename):
    if room_id not in ROOMS: return "Not Found", 404
    return send_from_directory(ROOMS[room_id]['path'], filename, as_attachment=True)


@app.route('/zip/<room_id>')
def download_zip(room_id):
    if room_id not in ROOMS: return "Not Found", 404
    memory_file = io.BytesIO()
    with zipfile.ZipFile(memory_file, 'w', zipfile.ZIP_DEFLATED) as zf:
        for root, _, files in os.walk(ROOMS[room_id]['path']):
            for file in files:
                zf.write(os.path.join(root, file), file)
    memory_file.seek(0)
    return send_file(memory_file, mimetype='application/zip', as_attachment=True, download_name=f'share_{room_id}.zip')


def open_browser(url):
    time.sleep(1)
    webbrowser.open(url)


if __name__ == '__main__':
    current_ip = get_real_lan_ip()
    url = f"http://{current_ip}:{PORT}"
    
    print("\n" + "="*60)
    print(f"🚀 WARP 局域网传输 (适配代理/TUN模式)")
    print(f"📡 监测到本机局域网 IP: {current_ip}")
    print(f"👉 电脑端请访问: {url}")
    print(f"📱 手机端扫描网页二维码即可下载")
    print("="*60 + "\n")

    threading.Thread(target=open_browser, args=(url,), daemon=True).start()
    
    app.run(host='0.0.0.0', port=PORT, debug=False)
