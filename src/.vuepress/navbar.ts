import { navbar } from "vuepress-theme-hope";

export default navbar([
  "/",
  "/demo/",
  {
    text: "博文",
    icon: "pen-to-square",
    prefix: "/posts/",
    children: [
      {
        text: "C++",
        icon: "pen-to-square",
        prefix: "c++/",
        children: [
          //下面还是文件夹 
          // 子菜单
          // 这里的link是相对于prefix的路径
          {
            text: "基础",
            icon: "pen-to-square",
            prefix: "基础/",
            children: [
              {
                text: "cpp98",
                icon: "pen-to-square",
                link: "01_cpp98_complete_guide", // 链接,或者md的名称
              },
              {
                text: "现代c++新特性",
                icon: "pen-to-square",
                link: "02_modern_cpp_features", // 链接,或者md的名称
              },
              {
                text: "文件操作和io",
                icon: "pen-to-square",
                link: "03_file_io_and_streams", // 链接,或者md的名称
              },
            ],
          },
          {
            text: "qt",
            icon: "pen-to-square",
            prefix: "qt/",
            children: [
              {
                text: "qt教程",
                icon: "pen-to-square",
                link: "qt_detailed_study_guide", // 链接,或者md的名称
              },
            ],
          },
        ],
      },
      // "tomato",
      {
        text: "虚幻",
        icon: "pen-to-square",
        prefix: "unreal/",// 子菜单
        children: [
          {
            text: "基础知识1",
            icon: "pen-to-square",
            link: "BasicUC++", // 链接,或者md的名称
          },
        ],
      },
      {
        text: "java",
        icon: "pen-to-square",
        prefix: "Java/",// 子菜单
        children: [
          {
            text: "多线程-暂无",
            icon: "pen-to-square",
            link: "Lern", // 链接,或者md的名称
          },
        ],
      }
    ],
  }
]);
