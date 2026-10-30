import { navbar } from "vuepress-theme-hope";

export default navbar([
  "/",
  { text: "文章导航", icon: "book", link: "/posts/" },
  { text: "网络编程", icon: "network-wired", link: "/posts/C__/io-multiplexing/" },
  { text: "关于", icon: "circle-info", link: "/intro.html" },
]);
