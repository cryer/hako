import subprocess
import tkinter as tk
from tkinter import ttk
import os
import sys
from collections import defaultdict
import re


class GitCommit:
    """表示一个Git提交"""
    def __init__(self, commit_hash, short_hash, parents, message, author, date):
        self.hash = commit_hash
        self.short_hash = short_hash
        self.parents = parents
        self.message = message
        self.author = author
        self.date = date
        self.branches = []
        self.is_head = False


class GitTreeVisualizer:
    """Git提交树可视化器"""
    
    def __init__(self, repo_path='.'):
        self.repo_path = repo_path
        self.commits = {}
        self.branches = {}
        self.head_branch = None
        self.root = None
        self.canvas = None
        
        # 为每个分支分配不同的颜色
        self.branch_colors = {}
        self.color_palette = [
            '#FF5722',  # 深橙 - master
            '#4CAF50',  # 绿色
            '#2196F3',  # 蓝色
            '#FFC107',  # 琥珀色
            '#9C27B0',  # 紫色
            '#E91E63',  # 粉红
            '#00BCD4',  # 青色
            '#FF9800',  # 橙色
            '#795548',  # 棕色
            '#607D8B',  # 蓝灰
        ]
        
        # 布局参数
        self.node_width = 140
        self.node_height = 70
        self.x_spacing = 250
        self.y_spacing = 100
        
    def run_git_command(self, cmd):
        """执行git命令"""
        try:
            result = subprocess.run(
                cmd,
                cwd=self.repo_path,
                capture_output=True,
                text=True,
                check=True
            )
            return result.stdout.strip()
        except subprocess.CalledProcessError as e:
            print(f"Git命令执行失败: {e}")
            return ""
    
    def parse_all_commits(self):
        """解析所有提交"""
        # 获取所有引用的提交
        log_format = "%H|%h|%P|%s|%an|%ad|%d"
        cmd = [
            'git', 'log', '--all', '--decorate=full', '--date=short',
            f'--pretty=format:{log_format}'
        ]
        
        output = self.run_git_command(cmd)
        if not output:
            return
            
        for line in output.split('\n'):
            if not line:
                continue
            parts = line.split('|')
            if len(parts) >= 6:
                commit_hash = parts[0]
                short_hash = parts[1]
                parents = parts[2].split() if parts[2] else []
                message = parts[3]
                author = parts[4]
                date = parts[5]
                refs = parts[6] if len(parts) > 6 else ""
                
                if commit_hash not in self.commits:
                    self.commits[commit_hash] = GitCommit(
                        commit_hash, short_hash, parents, message, author, date
                    )
                
                # 解析引用（分支和标签）
                if refs:
                    # 提取分支名
                    ref_matches = re.findall(r'ref: refs/heads/([^,\)]+)|HEAD -> ([^,\)]+)|([^,\)]+)', refs)
                    for match in ref_matches:
                        for group in match:
                            if group and not group.startswith('refs/'):
                                branch_name = group.strip()
                                if branch_name and branch_name not in self.commits[commit_hash].branches:
                                    self.commits[commit_hash].branches.append(branch_name)
                                    if commit_hash not in self.branches.values() or branch_name not in self.branches:
                                        self.branches[branch_name] = commit_hash
    
    def parse_branches_detail(self):
        """详细解析所有分支"""
        # 获取当前分支
        self.head_branch = self.run_git_command(['git', 'rev-parse', '--abbrev-ref', 'HEAD'])
        
        # 获取所有本地分支及其提交
        branches_output = self.run_git_command(['git', 'branch', '-v', '--all'])
        
        color_idx = 0
        for line in branches_output.split('\n'):
            if not line:
                continue
            
            # 解析分支行，处理不同的格式
            match = re.match(r'^[\*\s]\s+(\S+)\s+([0-9a-f]+)', line)
            if match:
                branch_name = match.group(1)
                commit_hash = match.group(2)
                
                # 为分支分配颜色
                if branch_name not in self.branch_colors:
                    self.branch_colors[branch_name] = self.color_palette[color_idx % len(self.color_palette)]
                    color_idx += 1
                
                self.branches[branch_name] = commit_hash
                
                # 标记提交对应的分支
                if commit_hash in self.commits:
                    if branch_name not in self.commits[commit_hash].branches:
                        self.commits[commit_hash].branches.append(branch_name)
                    if branch_name == self.head_branch:
                        self.commits[commit_hash].is_head = True
    
    def calculate_layout(self):
        """计算提交树的布局"""
        if not self.commits:
            return {}
        
        # 找到所有没有子节点的提交（tips）
        has_parent = set()
        for commit in self.commits.values():
            has_parent.update(commit.parents)
        
        # tips是那些没有作为父节点出现的提交
        tips = set(self.commits.keys()) - has_parent
        
        # 如果没有找到tips，使用分支指向的提交
        if not tips:
            tips = set(self.branches.values())
        
        # 使用BFS计算每个提交的层级和水平位置
        positions = {}
        visited = set()
        
        # 计算层级（从tips向下）
        def calculate_levels(commit_hash, level=0):
            if commit_hash not in self.commits or commit_hash in visited:
                return
            
            visited.add(commit_hash)
            
            commit = self.commits[commit_hash]
            
            # 为这个提交分配位置
            if level not in positions:
                positions[level] = []
            
            # 检查是否已在这个层级
            if commit_hash not in [p[0] for p in positions[level]]:
                positions[level].append((commit_hash, level))
            
            # 处理父节点
            for parent in commit.parents:
                calculate_levels(parent, level + 1)
        
        # 从所有tips开始
        for tip in tips:
            calculate_levels(tip, 0)
        
        # 重新组织位置信息
        level_commits = defaultdict(list)
        visited.clear()
        
        def assign_positions(commit_hash, level=0):
            if commit_hash not in self.commits or commit_hash in visited:
                return
            
            visited.add(commit_hash)
            level_commits[level].append(commit_hash)
            
            commit = self.commits[commit_hash]
            for parent in commit.parents:
                assign_positions(parent, level + 1)
        
        for tip in tips:
            assign_positions(tip, 0)
        
        # 计算每个提交的具体坐标
        commit_positions = {}
        for level in sorted(level_commits.keys()):
            commits_at_level = level_commits[level]
            total_width = len(commits_at_level) * self.x_spacing
            start_x = (1000 - total_width) // 2 if total_width < 1000 else 50
            
            for i, commit_hash in enumerate(commits_at_level):
                x = start_x + i * self.x_spacing
                y = level * self.y_spacing + 80
                commit_positions[commit_hash] = (x, y, level)
        
        return commit_positions
    
    def create_ui(self):
        """创建UI"""
        self.root = tk.Tk()
        self.root.title("Git Commit Tree - All Branches")
        self.root.geometry("1400x900")
        
        # 创建控制面板
        control_frame = ttk.Frame(self.root)
        control_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(control_frame, text="Git提交树可视化", font=("Arial", 14, "bold")).pack(side=tk.LEFT)
        
        # 创建图例
        self.create_legend(control_frame)
        
        # 创建Canvas和滚动条
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        v_scrollbar = ttk.Scrollbar(main_frame, orient=tk.VERTICAL)
        h_scrollbar = ttk.Scrollbar(main_frame, orient=tk.HORIZONTAL)
        
        self.canvas = tk.Canvas(
            main_frame,
            bg='#FAFAFA',
            highlightthickness=0,
            yscrollcommand=v_scrollbar.set,
            xscrollcommand=h_scrollbar.set
        )
        
        v_scrollbar.config(command=self.canvas.yview)
        h_scrollbar.config(command=self.canvas.xview)
        
        self.canvas.grid(row=0, column=0, sticky='nsew')
        v_scrollbar.grid(row=0, column=1, sticky='ns')
        h_scrollbar.grid(row=1, column=0, sticky='ew')
        
        main_frame.grid_rowconfigure(0, weight=1)
        main_frame.grid_columnconfigure(0, weight=1)
        
        # 绑定事件
        self.canvas.bind('<ButtonPress-1>', self.start_scroll)
        self.canvas.bind('<B1-Motion>', self.scroll)
        self.canvas.bind('<MouseWheel>', self.on_mousewheel)
        self.canvas.bind('<Button-4>', self.on_mousewheel)
        self.canvas.bind('<Button-5>', self.on_mousewheel)
        
    def create_legend(self, parent):
        """创建图例"""
        legend_frame = ttk.LabelFrame(parent, text="分支图例", padding=5)
        legend_frame.pack(side=tk.RIGHT, padx=10)
        
        for branch_name, color in self.branch_colors.items():
            frame = ttk.Frame(legend_frame)
            frame.pack(side=tk.LEFT, padx=5)
            
            canvas = tk.Canvas(frame, width=20, height=20, bg=color, highlightthickness=0)
            canvas.pack(side=tk.LEFT)
            
            display_name = branch_name.split('/')[-1] if len(branch_name) > 20 else branch_name
            ttk.Label(frame, text=display_name, font=("Arial", 8)).pack(side=tk.LEFT, padx=2)
    
    def start_scroll(self, event):
        self.canvas.scan_mark(event.x, event.y)
    
    def scroll(self, event):
        self.canvas.scan_dragto(event.x, event.y, gain=1)
    
    def on_mousewheel(self, event):
        if event.num == 4 or event.delta > 0:
            self.canvas.yview_scroll(-1, "units")
        elif event.num == 5 or event.delta < 0:
            self.canvas.yview_scroll(1, "units")
    
    def draw_tree(self):
        """绘制提交树"""
        positions = self.calculate_layout()
        
        if not positions:
            tk.Label(self.canvas, text="未找到提交记录", font=("Arial", 16)).pack(pady=50)
            return
        
        # 计算画布大小
        max_x = max(p[0] for p in positions.values())
        max_y = max(p[1] for p in positions.values())
        
        canvas_width = max(1400, max_x + 200)
        canvas_height = max(900, max_y + 200)
        
        self.canvas.config(scrollregion=(0, 0, canvas_width, canvas_height))
        
        # 先绘制连接线
        for commit_hash, pos in positions.items():
            x, y, _ = pos
            commit = self.commits[commit_hash]
            
            for parent_hash in commit.parents:
                if parent_hash in positions:
                    px, py, _ = positions[parent_hash]
                    self.draw_connection(x, y, px, py)
        
        # 再绘制节点
        for commit_hash, pos in positions.items():
            x, y, level = pos
            self.draw_commit_node(commit_hash, x, y)
    
    def draw_connection(self, x1, y1, x2, y2):
        """绘制连接线 - 修复箭头位置"""
        # 从子节点底部中心到父节点顶部中心
        start_x = x1
        start_y = y1 + self.node_height // 2
        
        end_x = x2
        end_y = y2 - self.node_height // 2
        
        # 绘制曲线
        mid_y = (start_y + end_y) // 2
        
        self.canvas.create_line(
            start_x, start_y,
            start_x, mid_y,
            end_x, mid_y,
            end_x, end_y,
            fill='#78909C',
            width=2,
            smooth=True
        )
        
        # 绘制箭头（在父节点顶部）
        arrow_size = 8
        self.canvas.create_polygon(
            end_x - arrow_size, end_y + arrow_size,
            end_x, end_y,
            end_x + arrow_size, end_y + arrow_size,
            fill='#78909C',
            outline='#78909C'
        )
    
    def draw_commit_node(self, commit_hash, x, y):
        """绘制提交节点"""
        commit = self.commits[commit_hash]
        
        # 确定节点颜色
        if commit.branches:
            # 使用第一个分支的颜色
            node_color = self.branch_colors.get(commit.branches[0], '#9E9E9E')
        else:
            node_color = '#757575'
        
        # 如果是HEAD，添加特殊标记
        if commit.is_head:
            node_color = '#FF5722'
        
        # 绘制节点背景（圆角矩形效果）
        self.canvas.create_rectangle(
            x - self.node_width // 2, y - self.node_height // 2,
            x + self.node_width // 2, y + self.node_height // 2,
            fill=node_color,
            outline='#424242',
            width=3,
            tags=f"commit_{commit_hash}"
        )
        
        # 绘制提交hash
        self.canvas.create_text(
            x, y - 20,
            text=commit.short_hash,
            fill='white',
            font=('Courier', 11, 'bold'),
            tags=f"commit_{commit_hash}"
        )
        
        # 绘制提交信息
        msg = commit.message[:25] + '...' if len(commit.message) > 25 else commit.message
        self.canvas.create_text(
            x, y + 5,
            text=msg,
            fill='white',
            font=('Arial', 9),
            tags=f"commit_{commit_hash}"
        )
        
        # 绘制分支标签
        if commit.branches:
            branch_y = y - self.node_height // 2 - 15
            for i, branch in enumerate(commit.branches):
                branch_color = self.branch_colors.get(branch, '#9C27B0')
                
                # 分支标签背景
                label_width = 60
                label_height = 18
                
                self.canvas.create_rectangle(
                    x - label_width // 2, 
                    branch_y + i * (label_height + 2) - label_height // 2,
                    x + label_width // 2, 
                    branch_y + i * (label_height + 2) + label_height // 2,
                    fill=branch_color,
                    outline='white',
                    width=2,
                    tags=f"branch_{branch}"
                )
                
                # 分支名称
                display_name = branch.split('/')[-1] if '/' in branch else branch
                if branch == self.head_branch:
                    display_name += " (HEAD)"
                
                self.canvas.create_text(
                    x, branch_y + i * (label_height + 2),
                    text=display_name,
                    fill='white',
                    font=('Arial', 8, 'bold'),
                    tags=f"branch_{branch}"
                )
        
        # 绑定事件
        self.canvas.tag_bind(
            f"commit_{commit_hash}",
            '<Enter>',
            lambda e, c=commit: self.show_tooltip(c, e.x_root, e.y_root)
        )
        self.canvas.tag_bind(
            f"commit_{commit_hash}",
            '<Leave>',
            lambda e: self.hide_tooltip()
        )
    
    def show_tooltip(self, commit, x, y):
        """显示工具提示"""
        self.tooltip = tk.Toplevel()
        self.tooltip.wm_overrideredirect(True)
        self.tooltip.wm_geometry(f"+{x+10}+{y+10}")
        
        text = f"Commit: {commit.short_hash}\n"
        text += f"Full: {commit.hash}\n"
        text += f"Author: {commit.author}\n"
        text += f"Date: {commit.date}\n"
        text += f"Message: {commit.message}\n"
        if commit.branches:
            text += f"Branches: {', '.join(commit.branches)}"
        
        label = tk.Label(
            self.tooltip,
            text=text,
            justify=tk.LEFT,
            background='#FFFFE0',
            relief=tk.SOLID,
            borderwidth=1,
            font=("Arial", 9),
            padx=8,
            pady=5
        )
        label.pack()
    
    def hide_tooltip(self):
        if hasattr(self, 'tooltip'):
            self.tooltip.destroy()
    
    def run(self):
        """运行可视化器"""
        print("正在解析Git仓库...")
        self.parse_all_commits()
        self.parse_branches_detail()
        
        print(f"找到 {len(self.commits)} 个提交")
        print(f"找到 {len(self.branches)} 个分支")
        
        for branch, commit in self.branches.items():
            print(f"  {branch} -> {commit[:8]}")
        
        if not self.commits:
            print("错误：未找到Git提交记录")
            return
        
        print("创建界面...")
        self.create_ui()
        self.draw_tree()
        
        print("完成！")
        self.root.mainloop()


def main():
    if not os.path.exists('.git'):
        print("错误：当前目录不是Git仓库")
        sys.exit(1)
    
    visualizer = GitTreeVisualizer('.')
    visualizer.run()


if __name__ == '__main__':
    main()
