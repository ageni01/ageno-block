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
          {
            text: "基础",
            icon: "pen-to-square",
            prefix: "基础/",
            link: "1",
          },
          {
            text: "qt",
            icon: "pen-to-square",
            link: "2",
            prefix: "qt/",
          },
          {
            text: "socket",
            icon: "pen-to-square",
            link: "3",
            prefix: "socket/",
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
